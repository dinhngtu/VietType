// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include <utility>
#include <cassert>
#include "Telex.h"
#include "TelexData.h"
#include "TelexEngine.h"

#define IS(cat, type) (static_cast<bool>((cat) & (type)))

namespace VietType {
namespace Telex {

static CharTypes ClassifyCharacter(_In_ wchar_t c) {
    if (c >= L'a' && c <= L'z') {
        return letterClasses[c - L'a'];
    }
    switch (c) {
    case L'\b':
        return CharTypes::Backspace;
    case L'\0':
        return CharTypes::ForceCommit;
    case L'\t':
    case L'\n':
    case L'\r':
    case L'|':
    case L'~':
        return CharTypes::Commit;
    }

    if (c >= 32 && c <= 64) {
        // ' ' to '@'
        return CharTypes::Commit;
    }
    if (c >= 91 && c <= 96) {
        return CharTypes::Commit;
    }

    return CharTypes::Uncategorized;
}

static Tones GetCharTone(_In_ wchar_t c) {
    switch (c) {
    case L'z':
        return Tones::Z;
    case L'f':
        return Tones::F;
    case L'j':
        return Tones::J;
    case L'r':
        return Tones::R;
    case L's':
        return Tones::S;
    case L'x':
        return Tones::X;
    default:
        return Tones::Z;
    }
}

// case functions hardcode ranges of Vietnamese characters
// the rest can be correctly transformed or not, doesn't matter

static wchar_t ToUpper(_In_ wchar_t c) {
    wchar_t uc = c & ~32;
    // Basic Latin
    if (uc >= L'A' && uc <= L'Z') {
        return uc;
    }
    // Latin-1 Supplement
    if (c >= L'\xe0' && c <= L'\xfe') {
        return uc;
    }
    // "uw" exception
    if (c >= L'\x1af' && c <= L'\x1b0') {
        return L'\x1af';
    }
    uc = c & ~1;
    // Latin Extended-A/B
    if (c >= L'\x100' && c <= L'\x1bf') {
        return uc;
    }
    // Latin Extended Additional
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        return uc;
    }
    return c;
}

static wchar_t ToLower(_In_ wchar_t c) {
    wchar_t lc = c | 32;
    if (lc >= L'a' && lc <= L'z') {
        return lc;
    }
    if (c >= L'\xc0' && c <= L'\xde') {
        return lc;
    }
    // "uw" exception
    if (c >= L'\x1af' && c <= L'\x1b0') {
        return L'\x1b0';
    }
    lc = c | 1;
    if (c >= L'\x100' && c <= L'\x1bf') {
        return lc;
    }
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        return lc;
    }
    return c;
}

// return 1 if c is upper, 0 if c is lower
static int FindLower(_In_ wchar_t c, _Out_ wchar_t* out) {
    *out = c | 32;
    if (*out >= L'a' && *out <= L'z') {
        return *out != c;
    }
    if (c >= L'\xe0' && c <= L'\xfe') {
        return *out != c;
    }
    // "uw" exception
    if (c >= L'\x1af' && c <= L'\x1b0') {
        return c == L'\x1af';
    }
    *out = c | 1;
    if (c >= L'\x100' && c <= L'\x1bf') {
        return *out != c;
    }
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        return *out != c;
    }
    *out = c;
    return 0;
}

static wchar_t TranslateTone(_In_ wchar_t c, _In_ Tones t) {
    auto it = transitions_tones.find(c);
    // don't fail here since tone position prediction might give invalid v
    if (it != transitions_tones.end()) {
        return (it->second)[(int)t];
    } else {
        return c;
    }
}

/// <summary>destructive</summary>
static void ApplyCases(_In_ std::wstring& str, _In_ const std::vector<int>& cases) {
    assert(str.length() == cases.size());
    for (size_t i = 0; i < cases.size(); i++) {
        if (cases[i]) {
            str[i] = ToUpper(str[i]);
        }
    }
}

struct TelexEngineImpl {
    template <typename T>
    static bool TransitionV(TelexEngine& e, const T& source, bool w_mode = false) {
        auto it = source.find(e._v);
        if (it != source.end() &&
            (!w_mode || ((e._v != it->second || e._c2.empty()) && !(e._respos.back() & ResposTransitionW)))) {
            e._v = it->second.str();
            return true;
        } else {
            return false;
        }
    }

    static inline void Invalidate(TelexEngine& e) {
        e._respos.push_back(e._respos_current++ | ResposInvalidate);
        e._state = TelexStates::Invalid;
    }

    static void InvalidateAndPopBack(TelexEngine& e, wchar_t c) {
        // pop back only if same char entered twice in a row
        if (c == ToLower(e._keyBuffer.rbegin()[1]))
            e._respos.push_back(e._respos_current++ | ResposDoubleUndo);
        else
            e._respos.push_back(e._respos_current++ | ResposInvalidate);
        e._state = TelexStates::Invalid;
    }

    static std::optional<std::pair<WConStr, VInfo>> FindTable(const TelexEngine& e) {
        if (e._c1 == L"q") {
            return valid_v_q.find_opt(e._v);
        } else if (e._c1 == L"gi") {
            return valid_v_gi.find_opt(e._v);
        } else {
            if (!e._c2.size() && !e._config.oa_uy_tone1) {
                auto it = valid_v_oa_uy.find(e._v);
                if (it != valid_v_oa_uy.end())
                    return *it;
            }
            return valid_v.find_opt(e._v);
        }
    }

    static bool GetTonePos(const TelexEngine& e, _In_ bool predict, _Out_ VInfo* vinfo) {
        auto found = FindTable(e);
        VInfo retinfo = {0, C2Mode::Either};
        if (found) {
            retinfo = found->second;
        } else if (predict) {
            // guess tone position if _v is not known
            switch (e._v.size()) {
            case 1:
                retinfo.tonepos = 0;
                retinfo.c2mode = C2Mode::Either;
                break;
            case 2:
            case 3:
                retinfo.tonepos = 1;
                retinfo.c2mode = C2Mode::Either;
                break;
            default:
                retinfo.tonepos = -1;
                retinfo.c2mode = C2Mode::Either;
                break;
            }
            if (e._c1 == L"q") {
                // quick fix to prevent pushing tone character when backspacing to 'qu'
                retinfo.tonepos = -1;
                retinfo.c2mode = C2Mode::Either;
            }
        }
        *vinfo = retinfo;
        return found.has_value();
    }

    static void ReapplyTone(TelexEngine& e) {
        e._toned = true;
        int found = -1;
        for (int i = static_cast<int>(e._respos.size() - 1); i >= 0; i--) {
            if (e._respos[i] & ResposTone) {
                found = i;
                break;
            }
        }
        if (found >= 0) {
            e._respos.push_back(found | ResposTone);
        } else {
            VInfo vinfo;
            if (TelexEngineImpl::GetTonePos(e, false, &vinfo))
                e._respos.push_back(static_cast<int>(e._c1.size() + vinfo.tonepos) | ResposTone);
            else
                e._respos.push_back(static_cast<int>(e._c1.size() + e._v.size() - 1) | ResposTone);
        }
    }
};

TelexEngine::TelexEngine(_In_ TelexConfig config) {
    _config = config;
    _keyBuffer.reserve(9);
    _c1.reserve(9);
    _v.reserve(9);
    _c2.reserve(9);
    _cases.reserve(9);
    _respos.reserve(9);
    Reset();
}

const TelexConfig& TelexEngine::GetConfig() const {
    return _config;
}

void TelexEngine::SetConfig(const TelexConfig& config) {
    _config = config;
}

void TelexEngine::Reset() {
    _state = TelexStates::Valid;
    _keyBuffer.clear();
    _c1.clear();
    _v.clear();
    _c2.clear();
    _t = Tones::Z;
    _toned = false;
    _cases.clear();
    _respos.clear();
    _respos_current = 0;
    _backconverted = false;
}

// remember to push into _cases when adding a new character
TelexStates TelexEngine::PushChar(_In_ wchar_t corig) {
    // PushChar at any committed/error state is illegal, but fail softly anyway
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid) {
        return _state;
    }

    _keyBuffer.push_back(corig);

    if (_state == TelexStates::Invalid || _keyBuffer.size() > 9) {
        TelexEngineImpl::Invalidate(*this);
        assert(CheckInvariants());
        return _state;
    }

    wchar_t c;
    auto ccase = FindLower(corig, &c);
    auto cat = ClassifyCharacter(c);
    if (cat == CharTypes::Uncategorized) {
        TelexEngineImpl::Invalidate(*this);

    } else if (_c1.empty() && _v.empty() && IS(cat, CharTypes::ConsoC1)) {
        // ConsoContinue is a subset of ConsoC1, no need to check
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_v.empty() && _c1 == L"g" && c == L'i') {
        // special treatment for 'gi'
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_c1 == L"d" && c == L'd' && (_config.accept_separate_dd || (_v.empty() && _c2.empty()))) {
        // only used for 'dd'
        // relaxed constraint: _v.empty()
        _c1 = L"\x111";
        _respos.push_back(0 | ResposTransitionC1);

    } else if (_c1 == L"\x111" && c == L'd') {
        // only used for 'dd'
        // relaxed constraint: _v.empty()
        if (_keyBuffer.size() > 1 && ToLower(_keyBuffer.rbegin()[1]) == L'd')
            _respos.push_back(_respos_current++ | ResposDoubleUndo);
        else
            _respos.push_back(_respos_current++ | ResposInvalidate);
        _state = TelexStates::Invalid;

    } else if (_v.empty() && _c2.empty() && _c1 != L"gi" && IS(cat, CharTypes::ConsoContinue)) {
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (IS(cat, CharTypes::Vowel)) {
        // relaxed vowel position constraint: _c2.empty()
        // vowel parts (aeiouy)
        _v.push_back(c);
        auto before = _v.size();
        if (TelexEngineImpl::TransitionV(*this, transitions)) {
            auto after = _v.size();
            if (_config.optimize_multilang >= 3 && _toned) {
                TelexEngineImpl::Invalidate(*this);
            } else if (
                _keyBuffer.size() > 1 && _respos.back() & ResposTransitionV && c == ToLower(_keyBuffer.rbegin()[1])) {
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++ | ResposDoubleUndo);
            } else if (after < before) {
                _respos.push_back(static_cast<int>(_c1.size() + _v.size() - 1) | ResposTransitionV);
            } else if (after == before) {
                // in case of 'uơi' -> 'ươi', the transition char itself is a normal character
                // so it must be recorded as such rather than just a transition
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++ | ResposTransitionV);
            }
        } else {
            // if there is no transition, there must be a new character -> must push case
            _cases.push_back(ccase);
            // invalidate if same char entered twice in a row in order to undo transition
            if (_keyBuffer.size() > 1 && _respos.back() & ResposTransitionV && c == ToLower(_keyBuffer.rbegin()[1])) {
                _respos.push_back(_respos_current++ | ResposDoubleUndo);
                _state = TelexStates::Invalid;
            } else {
                _respos.push_back(_respos_current++);
            }
            if (!_c2.empty()) {
                // in case there exists no transition when _c2 is already typed
                // e.g. 'cace'
                _state = TelexStates::Invalid;
            }
        }

    } else if (!_v.empty() && IS(cat, CharTypes::VowelW)) {
        bool tw;
        if (_c1 == L"q") {
            tw = TelexEngineImpl::TransitionV(*this, transitions_w_q, true);
        } else {
            tw = TelexEngineImpl::TransitionV(*this, transitions_w, true);
        }
        if (tw) {
            if (!_c2.empty()) {
                TelexEngineImpl::TransitionV(*this, transitions_v_c2);
            }
            _respos.push_back(static_cast<int>(_c1.size() + _v.size() - 1) | ResposTransitionW);
        } else {
            TelexEngineImpl::InvalidateAndPopBack(*this, c);
        }
        // 'w' always keeps V size constant, don't push case

    } else if ((_c1 == L"gi" || !_v.empty()) && IS(cat, CharTypes::Tone)) {
        // tones
        if (_config.optimize_multilang >= 3 && _toned) {
            TelexEngineImpl::Invalidate(*this);
        } else {
            auto newtone = GetCharTone(c);
            if (newtone != _t) {
                _t = newtone;
                TelexEngineImpl::ReapplyTone(*this);
            } else {
                // the condition "_c1 == L"gi" || !_v.empty()" should ensure this already
                assert(_keyBuffer.length() > 1);
                TelexEngineImpl::InvalidateAndPopBack(*this, c);
            }
        }

    } else if (((_c1 == L"gi" && _v.empty()) || !_v.empty()) && _c2.empty() && IS(cat, CharTypes::ConsoC2)) {
        // word-ending consonants (cnpt)
        bool success = true;
        // special teencode exception
        if (_c1 != L"\x111" && _t != Tones::Z && _t != Tones::S && _t != Tones::J) {
            wchar_t tmpc2[2] = {c, 0};
            auto testtone = valid_c2.find(tmpc2);
            // all the c2 that share a prefix have the same restrict value
            // i.e. valid_c2["c"]->second == valid_c2["ch"]->second
            // so we should know from just the first character
            if (testtone != valid_c2.end() && testtone->second)
                success = false;
        }
        if (success) {
            if (_c1 == L"q") {
                TelexEngineImpl::TransitionV(*this, transitions_v_c2_q);
            } else {
                TelexEngineImpl::TransitionV(*this, transitions_v_c2);
            }
            _c2.push_back(c);
            _cases.push_back(ccase);
            _respos.push_back(_respos_current++);
        } else {
            TelexEngineImpl::Invalidate(*this);
        }

    } else if (_c2.size() && IS(cat, CharTypes::ConsoContinue)) {
        // consonant continuation (dgh)
        _c2.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else {
        TelexEngineImpl::Invalidate(*this);
    }

    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Backspace() {
    [[maybe_unused]] auto prevState = _state;
    std::wstring buf(_keyBuffer);
    std::vector<int> rp(_respos);

    if (_state == TelexStates::BackconvertFailed) {
        _keyBuffer.pop_back();
        TelexEngine emulate(GetConfig());
        if (emulate.Backconvert(_keyBuffer) == TelexStates::Valid) {
            *this = std::move(emulate);
        }
        return _state;
    } else if (_state == TelexStates::Invalid) {
        Reset();
        if (buf.size()) {
            buf.pop_back();
        }
        if (buf.size() && _config.backspaced_word_stays_invalid) {
            _state = TelexStates::Invalid;
        }
        for (size_t i = 0; i < buf.size(); i++)
            if (!(rp[i] & ResposDoubleUndo))
                PushChar(buf[i]);
        assert(CheckInvariantsBackspace(prevState));
        return _state;
    } else if (_state != TelexStates::Valid) {
        return TelexStates::TxError;
    }

    // if cannot set tone like in Peek, treat this as invalid (but do not mark word as invalid for further correction)
    VInfo vinfo;
    auto found = TelexEngineImpl::GetTonePos(*this, false, &vinfo);
    if (!found && _t != Tones::Z) {
        Reset();
        if (buf.size()) {
            buf.pop_back();
        }
        for (auto c : buf) {
            PushChar(c);
        }
        assert(CheckInvariantsBackspace(prevState));
        return _state;
    }

    assert(_keyBuffer.size() == _respos.size());
    rp = _respos;
    std::wstring oldc1(_c1);
    std::wstring oldv(_v);
    bool oldBackconverted = _backconverted;

    auto toDelete = static_cast<int>(_c1.size() + _v.size() + _c2.size()) - 1;

    Reset();

    // ensure only one key in the _keyBuffer is Tone
    int lastTone = -1;
    for (size_t i = 0; i < buf.size(); i++) {
        if (rp[i] & ResposTone) {
            lastTone = static_cast<int>(i);
            rp[i] = (rp[i] & ResposMask) | ResposExpunged;
        }
    }
    if (lastTone >= 0) {
        rp[lastTone] = (rp[lastTone] & ResposMask) | ResposTone;
    }

    for (size_t i = 0; i < buf.size(); i++) {
        if (rp[i] & ResposDoubleUndo && (rp[i] & ResposMask) >= toDelete) {
            assert(i > 0);
            assert(rp[i - 1] & ~ResposMask);
            rp[i - 1] = (rp[i - 1] & ResposMask) | ResposExpunged;
        }
    }

    for (size_t i = 0; i < buf.size(); i++)
        if (!(rp[i] & ResposExpunged) && (rp[i] & ResposMask) < toDelete)
            PushChar(buf[i]);

    if (_keyBuffer.size()) {
        _backconverted = oldBackconverted;
    }

    assert(CheckInvariantsBackspace(prevState));
    return _state;
}

TelexStates TelexEngine::Commit() {
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid ||
        _state == TelexStates::BackconvertFailed) {
        return _state;
    }

    if (_state == TelexStates::Invalid) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    if (!_keyBuffer.size()) {
        _state = TelexStates::Committed;
        return _state;
    }

    if (_state == TelexStates::Valid && _config.optimize_multilang >= 1) {
        auto wordBuffer = _keyBuffer;
        for (auto& c : wordBuffer) {
            c = ToLower(c);
        }
        if (word_exceptions_en.find(wordBuffer) != word_exceptions_en.end()) {
            _state = TelexStates::CommittedInvalid;
            return _state;
        }
        if (_config.optimize_multilang >= 2 && word_exceptions_en_2.find(wordBuffer) != word_exceptions_en_2.end()) {
            _state = TelexStates::CommittedInvalid;
            return _state;
        }
    }

    // validate c1
    auto c1_it = valid_c1.find(_c1);
    if (c1_it == valid_c1.end()) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // validate c2
    auto c2_it = valid_c2.find(_c2);
    if (c2_it == valid_c2.end()) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }
    if (c2_it->second && !(_t == Tones::S || _t == Tones::J)) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // validate v and get tone position
    VInfo vinfo;
    auto found = TelexEngineImpl::GetTonePos(*this, false, &vinfo);
    if (!found) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // routine changes buffers from this point

    if (vinfo.tonepos < 0 && _c1 == L"gi" && _v.empty()) {
        // fixup 'gi' by moving 'i' to _v
        _c1.pop_back();
        _v.push_back(L'i');
        vinfo.tonepos = 0;
    } else if (vinfo.c2mode == C2Mode::MustC2 && !_c2.size()) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    } else if (vinfo.c2mode == C2Mode::NoC2 && _c2.size()) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    _v[vinfo.tonepos] = TranslateTone(_v[vinfo.tonepos], _t);

    _state = TelexStates::Committed;
    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::ForceCommit() {
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid ||
        _state == TelexStates::BackconvertFailed) {
        return _state;
    }

    if (_state == TelexStates::Invalid) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    if (!_keyBuffer.size()) {
        _state = TelexStates::Committed;
        return _state;
    }

    VInfo vinfo;
    auto found = TelexEngineImpl::GetTonePos(*this, false, &vinfo);
    if (!found) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }
    _v[vinfo.tonepos] = TranslateTone(_v[vinfo.tonepos], _t);

    _state = TelexStates::Committed;
    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Cancel() {
    if (_backconverted && _c1.size() + _v.size() + _c2.size() != _keyBuffer.size()) {
        auto s = Peek();
        _keyBuffer = s;
        _state = TelexStates::BackconvertFailed;
    } else {
        _state = TelexStates::CommittedInvalid;
    }
    return _state;
}

TelexStates TelexEngine::Backconvert(_In_ const std::wstring& s) {
    assert(!_keyBuffer.size());
    bool found_backconversion = false;
    for (auto c : s) {
        auto double_flag = !_c2.size() && (_v == L"e" || _v == L"o");
        if (c >= L'a' && c <= L'z') {
            if (double_flag && c == _v[0])
                PushChar(c);
            PushChar(c);
        } else if (c >= L'A' && c <= L'Z') {
            if (double_flag && ToLower(c) == _v[0])
                PushChar(c);
            PushChar(c);
        } else {
            auto clow = ToLower(c);
            auto it = backconversions.find(clow);
            assert(it != backconversions.end());
            if (double_flag && it->second[0] == _v[0]) {
                if (c != clow) {
                    // c is upper
                    PushChar(ToUpper(it->second[0]));
                } else {
                    PushChar(it->second[0]);
                }
            }
            for (auto backc : it->second) {
                if (c != clow) {
                    // c is upper
                    PushChar(ToUpper(backc));
                } else {
                    PushChar(backc);
                }
            }
            found_backconversion = true;
        }
    }
    if (_c1.size() + _v.size() + _c2.size() != s.size()) {
        if (found_backconversion) {
            _keyBuffer = s;
            _state = TelexStates::BackconvertFailed;
        } else {
            _state = TelexStates::Invalid;
        }
    }
    if (_keyBuffer.size()) {
        _backconverted = true;
    }
    assert(CheckInvariants());
    return _state;
}

std::wstring TelexEngine::Retrieve() const {
    if (_state == TelexStates::Invalid || _state == TelexStates::CommittedInvalid ||
        _state == TelexStates::BackconvertFailed) {
        return RetrieveRaw();
    }
    std::wstring result(_c1);
    result.append(_v);
    result.append(_c2);
    ApplyCases(result, _cases);
    return result;
}

std::wstring TelexEngine::RetrieveRaw() const {
    std::wstring result;
    if (_state != TelexStates::BackconvertFailed) {
        for (size_t i = 0; i < _keyBuffer.size(); i++)
            if (!(_respos[i] & ResposDoubleUndo))
                result.push_back(_keyBuffer[i]);
    } else {
        result = _keyBuffer;
    }
    return result;
}

std::wstring TelexEngine::Peek() const {
    if (_state == TelexStates::Invalid || _state == TelexStates::CommittedInvalid ||
        _state == TelexStates::BackconvertFailed) {
        return RetrieveRaw();
    }

    std::wstring result(_c1);
    result.append(_v);

    VInfo vinfo;
    auto found = TelexEngineImpl::GetTonePos(*this, false, &vinfo);
    if (!found) {
        if (_t == Tones::Z) {
            result.append(_c2);
            ApplyCases(result, _cases);
            return result;
        } else {
            return RetrieveRaw();
        }
    }

    // fixup 'gi' then apply tone
    if (vinfo.tonepos < 0 && _c1 == L"gi" && _v.empty()) {
        vinfo.tonepos = (int)_c1.size() - 1;
        wchar_t vatpos = TranslateTone(_c1[vinfo.tonepos], _t);
        result[vinfo.tonepos] = vatpos;
    } else if (vinfo.tonepos >= 0) {
        wchar_t vatpos = TranslateTone(_v[vinfo.tonepos], _t);
        result[_c1.size() + vinfo.tonepos] = vatpos;
    }

    result.append(_c2);
    ApplyCases(result, _cases);

    return result;
}

bool TelexEngine::CheckInvariants() const {
    if (_state == TelexStates::TxError) {
        return false;
    }
    if (!_keyBuffer.size()) {
        if (_state != TelexStates::Valid)
            return false;
        if (_c1.size() || _v.size() || _c2.size())
            return false;
        if (_cases.size() || _respos.size() || _respos_current != 0)
            return false;
        if (_backconverted)
            return false;
    }
    if (_state != TelexStates::BackconvertFailed) {
        if (_c1.size() + _v.size() + _c2.size() > _keyBuffer.size())
            return false;
        if (_keyBuffer.size() != _respos.size())
            return false;
    }
    if (_state == TelexStates::Valid || _state == TelexStates::Committed) {
        if (_c1.size() + _v.size() + _c2.size() != _cases.size()) {
            return false;
        }
    }
    return true;
}

bool TelexEngine::CheckInvariantsBackspace(TelexStates prevState) const {
    if (prevState == TelexStates::Valid && _state != TelexStates::Valid) {
        return false;
    }
    return CheckInvariants();
}

} // namespace Telex
} // namespace VietType
