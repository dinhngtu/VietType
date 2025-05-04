// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include <utility>
#include <cassert>
#include <stdexcept>
#include <intrin.h>
#include "TelexMaps.h"
#include "TelexEngine.h"
#include "TelexData.h"

#define IS(cat, type) (!!static_cast<unsigned int>((cat) & (type)))

namespace VietType {
namespace Telex {

ITelexEngine* TelexNew(const TelexConfig& config) {
    return new TelexEngine(config);
}

void TelexDelete(ITelexEngine* engine) {
    delete engine;
}

constexpr size_t MaxLength = 10; // enough for "nghieengsz" and "nhuwowngxf"

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
    // Basic Latin
    if (lc >= L'a' && lc <= L'z') {
        return lc;
    }
    // Latin-1 Supplement
    if (c >= L'\xc0' && c <= L'\xde') {
        return lc;
    }
    // "uw" exception
    if (c >= L'\x1af' && c <= L'\x1b0') {
        return L'\x1b0';
    }
    lc = c | 1;
    // Latin Extended-A/B
    if (c >= L'\x100' && c <= L'\x1bf') {
        return lc;
    }
    // Latin Extended Additional
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        return lc;
    }
    return c;
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

static inline Tones GetCharTone(_In_ CharTypes cat) {
    assert(IS(cat, CharTypes::Tone));
    unsigned long bit;
    [[maybe_unused]] auto flag = _BitScanForward(&bit, (static_cast<unsigned int>(cat) >> 16));
    assert(flag && bit < 6);
    return static_cast<Tones>(bit);
}

inline CharTypes TelexEngine::ClassifyCharacter(_In_ wchar_t lc) const {
    const auto& ct = GetTypingStyle()->chartypes;
    if (lc >= std::size(ct))
        return CharTypes::Uncategorized;
    return ct[lc];
}

inline void TelexEngine::Invalidate() {
    _respos.push_back(_respos_current++ | ResposInvalidate);
    _state = TelexStates::Invalid;
}

void TelexEngine::InvalidateAndPopBack(wchar_t c) {
    // pop back only if same char entered twice in a row
    if (_keyBuffer.length() > 1 && c == ToLower(_keyBuffer.rbegin()[1]))
        _respos.push_back(_respos_current++ | ResposDoubleUndo);
    else
        _respos.push_back(_respos_current++ | ResposInvalidate);
    _state = TelexStates::Invalid;
}

std::optional<std::pair<std::wstring_view, VInfo>> TelexEngine::FindTable() const {
    if (_c1 == L"q") {
        return valid_v_q.find_opt(_v);
    } else if (_c1 == L"gi") {
        return valid_v_gi.find_opt(_v);
    } else {
        if (_c2.empty() && !_config.oa_uy_tone1) {
            auto it = valid_v_oa_uy.find(_v);
            if (it != valid_v_oa_uy.end())
                return *it;
        }
        return valid_v.find_opt(_v);
    }
}

bool TelexEngine::GetTonePos(_In_ bool predict, _Out_ VInfo* vinfo) const {
    auto found = FindTable();
    VInfo retinfo = {0, C2Mode::Either};
    if (found) {
        retinfo = found->second;
    } else if (predict) {
        // guess tone position if _v is not known
        switch (_v.size()) {
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
        if (_c1 == L"q") {
            // quick fix to prevent pushing tone character when backspacing to 'qu'
            retinfo.tonepos = -1;
            retinfo.c2mode = C2Mode::Either;
        }
    }
    *vinfo = retinfo;
    return found.has_value();
}

void TelexEngine::ReapplyTone() {
    _toneCount++;
    int found = -1;
    for (int i = static_cast<int>(_respos.size() - 1); i >= 0; i--) {
        if (_respos[i] & ResposTone) {
            found = i;
            break;
        }
    }
    if (found >= 0) {
        _respos.push_back(found | ResposTone);
    } else {
        VInfo vinfo;
        if (GetTonePos(false, &vinfo))
            _respos.push_back(static_cast<int>(_c1.size() + vinfo.tonepos) | ResposTone);
        else
            _respos.push_back(static_cast<int>(_c1.size() + _v.size() - 1) | ResposTone);
    }
}

bool TelexEngine::HasValidRespos() const {
    return std::any_of(_respos.begin(), _respos.end(), [](auto rp) { return rp & ResposValidMask; });
}

void TelexEngine::FeedNewResultChar(std::wstring& target, wchar_t c, bool ccase, int respos_flags) {
    target.push_back(c);
    _cases.push_back(ccase);
    _respos.push_back(_respos_current++ | respos_flags);
}

inline const TypingStyle* TelexEngine::GetTypingStyle() const {
    auto typing_style = static_cast<unsigned int>(_config.typing_style);
    assert(typing_style < typing_styles.size());
    return &typing_styles[typing_style];
}

TelexEngine::TelexEngine(const TelexConfig& config) {
    if (config.typing_style >= TypingStyles::Max) {
        throw std::invalid_argument("invalid typing style");
    }
    _config = config;
    _keyBuffer.reserve(MaxLength);
    _c1.reserve(MaxLength);
    _v.reserve(MaxLength);
    _c2.reserve(MaxLength);
    _cases.reserve(MaxLength);
    _respos.reserve(MaxLength);
    Reset();
}

const TelexConfig& TelexEngine::GetConfig() const {
    return _config;
}

void TelexEngine::SetConfig(const TelexConfig& config) {
    if (config.typing_style >= TypingStyles::Max) {
        throw std::invalid_argument("invalid typing style");
    }
    auto last = _config.typing_style;
    _config = config;
    if (last != config.typing_style) {
        Reset();
    }
}

void TelexEngine::Reset() {
    _state = TelexStates::Valid;
    _keyBuffer.clear();
    _c1.clear();
    _v.clear();
    _c2.clear();
    _t = Tones::Z;
    _toneCount = 0;
    _cases.clear();
    _respos.clear();
    _respos_current = 0;
    _backconverted = false;
    _autocorrected = false;
    assert(CheckInvariants());
}

// remember to push into _cases when adding a new character
TelexStates TelexEngine::PushChar(wchar_t corig) {
    // PushChar at any committed/error state is illegal, but fail softly anyway
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid) {
        return _state;
    }
    // don't let respos overflow into flags
    if (_keyBuffer.size() > 250) {
        _state = TelexStates::Invalid;
        assert(CheckInvariants());
        return _state;
    }

    _keyBuffer.push_back(corig);

    if (_state == TelexStates::Invalid || _keyBuffer.size() > MaxLength) {
        Invalidate();
        assert(CheckInvariants());
        return _state;
    }

    wchar_t c = ToLower(corig);
    auto ccase = c != corig;
    auto cat = ClassifyCharacter(c);
    if (cat == CharTypes::Uncategorized) {
        Invalidate();

    } else if (_c1.empty() && _v.empty() && IS(cat, CharTypes::ConsoC1)) {
        // ConsoContinue is a subset of ConsoC1, no need to check
        FeedNewResultChar(_c1, c, ccase);

    } else if (_v.empty() && _c1 == L"g" && c == L'i') {
        // special treatment for 'gi'
        FeedNewResultChar(_c1, c, ccase);

    } else if (_c1 == L"d" && IS(cat, CharTypes::Dd) && (_config.accept_separate_dd || (_v.empty() && _c2.empty()))) {
        // only used for 'dd'
        _c1 = L"\x111";
        _respos.push_back(0 | ResposTransitionC1);

    } else if (_c1 == L"\x111" && IS(cat, CharTypes::Dd)) {
        // only used for 'dd'
        // relaxed constraint: _v.empty()
        InvalidateAndPopBack(c);

    } else if (_v.empty() && _c2.empty() && _c1 != L"gi" && IS(cat, CharTypes::ConsoContinue)) {
        FeedNewResultChar(_c1, c, ccase);

    } else if (IS(cat, CharTypes::Vowel | CharTypes::Transition)) {
        // relaxed vowel position constraint: _c2.empty()
        _v.push_back(c);
        auto before = _v.size();
        // HACK: single special case for "khongoo"
        // note that _v here is post-append but pre-transition
        if (!_c2.empty() && _config.typing_style == TypingStyles::Telex && c == L'o' && _v == L"\xf4o") {
            Invalidate();
        } else if (TransitionV(GetTypingStyle()->transitions)) {
            auto after = _v.size();
            if (GetOptimizeLevel() >= 3 && _toneCount) {
                Invalidate();
            } else if (
                _keyBuffer.size() > 1 && _respos.back() & ResposTransitionV && c == ToLower(_keyBuffer.rbegin()[1])) {
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++ | ResposDoubleUndo);
            } else if (after < before) {
                // make sure transitions will only consume the typed character in this case
                assert(after == before - 1);
                _respos.push_back(static_cast<int>(_c1.size() + _v.size() - 1) | ResposTransitionV);
            } else if (after == before) {
                // in case of 'uơi' -> 'ươi', the transition char itself is a normal character
                // so it must be recorded as such rather than just a transition
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++ | ResposTransitionV);
            }
        } else if (IS(cat, CharTypes::Vowel)) {
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
        } else {
            _v.pop_back();
            InvalidateAndPopBack(c);
        }

    } else if (IS(cat, CharTypes::W | CharTypes::WA)) {
        if (!_v.empty()) {
            bool vw_transitioned = false;
            if (IS(cat, CharTypes::W)) {
                vw_transitioned = TransitionV(_c1 == L"q" ? transitions_w_q : transitions_w, true);
            }
            if (!vw_transitioned && IS(cat, CharTypes::WA)) {
                // with the dual-action "w" in telex, CharTypes::W takes priority
                vw_transitioned = TransitionV(_c1 == L"q" ? transitions_wa_q : transitions_wa, true);
            }
            if (vw_transitioned) {
                if (!_c2.empty()) {
                    TransitionV(_c1 == L"q" ? transitions_wv_c2_q : transitions_wv_c2);
                }
                _respos.push_back(static_cast<int>(_c1.size() + _v.size() - 1) | ResposTransitionW);
            } else {
                InvalidateAndPopBack(c);
            }
            // 'w' always keeps V size constant, don't push case
        } else if (
            _config.typing_style == TypingStyles::Telex && _config.autocorrect && !_toneCount &&
            (!_c1.empty() || GetOptimizeLevel() == 0)) {
            // at >=1 optimization, autocorrecting "nwuocs" is desirable but "wuocs" not
            FeedNewResultChar(_v, c, ccase, ResposAutocorrect);
        } else {
            Invalidate();
        }

    } else if ((_c1 == L"gi" || !_v.empty()) && IS(cat, CharTypes::Tone)) {
        // tones
        auto newtone = GetCharTone(cat);
        if (newtone != _t) {
            if (GetOptimizeLevel() >= 3 && _toneCount) {
                Invalidate();
            } else {
                _t = newtone;
                ReapplyTone();
            }
        } else {
            InvalidateAndPopBack(c);
        }

    } else if ((_c1 == L"gi" || !_v.empty()) && _c2.empty() && IS(cat, CharTypes::ConsoC2)) {
        // word-ending consonants (cnpt)
        bool success = true;
        // special teencode exception
        if (_c1 != L"\x111" && _t != Tones::Z && _t != Tones::S && _t != Tones::J) {
            wchar_t tmpc2[2] = {c, 0};
            auto testtone = valid_c2.find(tmpc2);
            // all the c2 that share a prefix have the same restrict value
            // so we should know from just the first character
            if (testtone != valid_c2.end() && testtone->second)
                success = false;
        }
        if (success) {
            TransitionV(_c1 == L"q" ? transitions_wv_c2_q : transitions_wv_c2);
            FeedNewResultChar(_c2, c, ccase);
        } else {
            Invalidate();
        }

    } else if (!_c2.empty() && IS(cat, CharTypes::ConsoContinue)) {
        // consonant continuation (dgh)
        FeedNewResultChar(_c2, c, ccase);

    } else if ((_c1 == L"gi" || !_v.empty()) && IS(cat, CharTypes::Conso)) {
        // special case for delaying the invalidation of invalid c2 until commit
        if (_c2.empty()) {
            TransitionV(_c1 == L"q" ? transitions_wv_c2_q : transitions_wv_c2);
        }
        FeedNewResultChar(_c2, c, ccase);

    } else {
        Invalidate();
    }

    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Backspace() {
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid && _state != TelexStates::BackconvertFailed) {
        return _state;
    }

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
        if (!rp.empty() && rp.back() & ResposDoubleUndo) {
            buf.pop_back();
        }
        if (!buf.empty()) {
            buf.pop_back();
        }
        if (!buf.empty() && _config.backspaced_word_stays_invalid) {
            _state = TelexStates::Invalid;
        }
        for (size_t i = 0; i < buf.size(); i++)
            if (!_config.backspaced_word_stays_invalid || !(rp[i] & ResposDoubleUndo))
                // if backspaced_word_stays_invalid=1, we need to push all chars in order to reproduce the
                // ResposDoubleUndo, thus the check
                PushChar(buf[i]);
        assert(CheckInvariantsBackspace(prevState));
        return _state;
    } else if (_state != TelexStates::Valid) {
        return TelexStates::TxError;
    }

    // if cannot set tone like in Peek, treat this as invalid (but do not mark word as invalid for further correction)
    VInfo vinfo;
    auto found = GetTonePos(false, &vinfo);
    if (!found && _t != Tones::Z) {
        Reset();
        if (!buf.empty()) {
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

    // scan word for respos that should be expunged
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

    if (!_keyBuffer.empty()) {
        _backconverted = oldBackconverted;
    }

    assert(CheckInvariantsBackspace(prevState));
    return _state;
}

TelexStates TelexEngine::DoOptimizeAndAutocorrect() {
    // precondition
    assert(_state == TelexStates::Valid);

    if (GetOptimizeLevel() >= 1) {
        std::wstring wordBuffer = _keyBuffer;
        for (auto& c : wordBuffer) {
            c = ToLower(c);
        }
        if (wlist_en.find(wordBuffer) != wlist_en.end()) {
            _state = TelexStates::CommittedInvalid;
            assert(CheckInvariants());
            return _state;
        }
        if (_config.autocorrect && wlist_en_ac.find(wordBuffer) != wlist_en_ac.end()) {
            _state = TelexStates::CommittedInvalid;
            assert(CheckInvariants());
            return _state;
        }
        if (GetOptimizeLevel() >= 2 && wlist_en_2.find(wordBuffer) != wlist_en_2.end()) {
            _state = TelexStates::CommittedInvalid;
            assert(CheckInvariants());
            return _state;
        }
    }

    if (_config.autocorrect && !_backconverted && _toneCount < 2) {
        // fixing respos might not be necessary here but fixing cases is
        // HACK
        if (_config.typing_style == TypingStyles::Telex) {
            if (_v == L"wu") {
                _v = L"\x1b0u";
                _autocorrected = true;
            } else if (!_c1.empty() && _v == L"wo") {
                _v = L"\x1a1";
                for (auto& rp : _respos)
                    if (rp & ResposAutocorrect)
                        _cases.erase(_cases.begin() + (rp & ResposMask));
                _autocorrected = true;
            } else if (_v == L"wuo") {
                _v = L"\x1b0\x1a1";
                for (auto& rp : _respos)
                    if (rp & ResposAutocorrect)
                        _cases.erase(_cases.begin() + (rp & ResposMask));
                _autocorrected = true;
            }
        }
        if (!_c1.empty() && _v == L"ie" && !_c2.empty() && (_t == Tones::S || _t == Tones::J)) {
            _v = L"i\xea";
            _autocorrected = true;
        }
        if (_c2 == L"h" && (_v == L"a" || _v == L"\xea")) {
            if (_t == Tones::S || _t == Tones::J) {
                _c2 = L"ch";
                _cases.push_back(_cases[_c1.length() + _v.length()]);
                _autocorrected = true;
            } else if (GetOptimizeLevel() <= 1 || HasValidRespos()) {
                _c2 = L"nh";
                _cases.push_back(_cases[_c1.length() + _v.length()]);
                _autocorrected = true;
            }
        }
        if (HasValidRespos()) {
            if (_c2 == L"gn") {
                _c2 = L"ng";
                _autocorrected = true;
            } else if (GetOptimizeLevel() <= 1 && _c2 == L"g") {
                _c2 = L"ng";
                _cases.push_back(_cases.back());
                _autocorrected = true;
            }
        }
    }

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

    if (_keyBuffer.empty()) {
        _state = TelexStates::Committed;
        return _state;
    }

    if (_state == TelexStates::Valid && DoOptimizeAndAutocorrect() != TelexStates::Valid) {
        return _state;
    }

    // validate c1
    auto c1_it = valid_c1.find(_c1);
    if (c1_it == valid_c1.end()) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
        return _state;
    }

    // validate c2
    auto c2_it = valid_c2.find(_c2);
    if (c2_it == valid_c2.end()) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
        return _state;
    }
    if (c2_it->second && !(_t == Tones::S || _t == Tones::J)) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
        return _state;
    }

    // validate v and get tone position
    VInfo vinfo;
    auto found = GetTonePos(false, &vinfo);
    if (!found) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
        return _state;
    }

    // routine changes buffers from this point

    if (vinfo.tonepos < 0 && _c1 == L"gi" && _v.empty()) {
        // fixup 'gi' by moving 'i' to _v
        _c1.pop_back();
        _v.push_back(L'i');
        vinfo.tonepos = 0;
    } else if (vinfo.c2mode == C2Mode::MustC2 && _c2.empty()) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
        return _state;
    } else if (vinfo.c2mode == C2Mode::NoC2 && !_c2.empty()) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
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

    if (_keyBuffer.empty()) {
        _state = TelexStates::Committed;
        return _state;
    }

    VInfo vinfo;
    auto found = GetTonePos(false, &vinfo);
    if (!found) {
        _state = TelexStates::CommittedInvalid;
        assert(CheckInvariants());
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
    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Backconvert(const std::wstring& s) {
    assert(_keyBuffer.empty());
    if (!_keyBuffer.empty())
        return _state;
    bool found_backconversion = false;
    bool failed = false;
    for (auto c : s) {
        // for emulating double key outcomes ("xoong")
        auto double_flag = _config.typing_style == TypingStyles::Telex && _c2.empty() && (_v == L"e" || _v == L"o");
        auto clow = ToLower(c);
        auto cat = ClassifyCharacter(clow);
        if (cat != CharTypes::Uncategorized) {
            if (double_flag && clow == _v[0])
                PushChar(c);
            PushChar(c);
        } else {
            auto it = GetTypingStyle()->backconversions.find(clow);
            if (it != GetTypingStyle()->backconversions.end()) {
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
            } else {
                failed = true;
            }
        }
    }
    if (failed || (_c1.size() + _v.size() + _c2.size() != s.size())) {
        if (failed || found_backconversion) {
            _keyBuffer = s;
            _state = TelexStates::BackconvertFailed;
        } else {
            _state = TelexStates::Invalid;
        }
    }
    if (!_keyBuffer.empty()) {
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
    auto found = GetTonePos(false, &vinfo);
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

bool TelexEngine::AcceptsChar(wchar_t c) const {
    const auto& charlist = GetTypingStyle()->charlist;
    return charlist.find(ToLower(c)) != std::wstring_view::npos;
}

bool TelexEngine::CheckInvariants() const {
    if (_state == TelexStates::TxError) {
        return false;
    }
    if (_keyBuffer.empty()) {
        if (_state != TelexStates::Valid && _state != TelexStates::Committed && _state != TelexStates::CommittedInvalid)
            // CommittedInvalid might be caused by Cancel()
            return false;
        if (!(_c1.empty() && _v.empty() && _c2.empty()))
            return false;
        if (_t != Tones::Z || _toneCount > 0)
            return false;
        if (!(_cases.empty() && _respos.empty() && _respos_current == 0))
            return false;
        if (_backconverted)
            return false;
    }
    // ResposExpunged is not meant to survive beyond Backspace()
    if (std::any_of(_respos.begin(), _respos.end(), [](auto r) { return r & ResposExpunged; }))
        return false;
    if (_state == TelexStates::Valid || _state == TelexStates::Invalid) {
        if (_c1.size() + _v.size() + _c2.size() > _keyBuffer.size())
            return false;
    }
    if (_state == TelexStates::Valid) {
        if (_keyBuffer.size() != _respos.size())
            return false;
    }
    if (_state == TelexStates::Valid || _state == TelexStates::Committed) {
        if (_c1.size() + _v.size() + _c2.size() != _cases.size())
            return false;
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
