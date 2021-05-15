// SPDX-License-Identifier: GPL-3.0-only

#include "Common.h"
#include "TelexEngine.h"
#include "TelexData.h"
#include "TelexUtil.h"

#define IS(cat, type) (static_cast<bool>((cat) & (type)))

namespace VietType {
namespace Telex {

enum ResposTransitions {
    ResposExpunged = -1,
    ResposTransitionC1 = -2,
    ResposTransitionV = -3,
    ResposTransitionW = -4,
    ResposTone = -5,
};

// case functions hardcode ranges of Vietnamese characters
// the rest can be correctly transformed or not, doesn't matter

static wchar_t ToUpper(_In_ wchar_t c) {
    auto uc = c & ~32;
    // Basic Latin
    if (uc >= L'A' && uc <= L'Z') {
        return uc;
    }
    // Latin-1 Supplement
    if (c >= L'\xc0' && c <= L'\xde') {
        return uc;
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
    auto lc = c | 32;
    if (lc >= L'a' && lc <= L'z') {
        return lc;
    }
    if (c >= L'\xe0' && c <= L'\xfe') {
        return lc;
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
    _cases.clear();
    _respos.clear();
    _respos_current = 0;
}

// remember to push into _cases when adding a new character
TelexStates TelexEngine::PushChar(_In_ wchar_t corig) {
    // PushChar at any committed/error state is illegal, but fail softly anyway
    if (_state != TelexStates::Valid && _state != TelexStates::Invalid) {
        return _state;
    }

    _keyBuffer.push_back(corig);

    if (_state == TelexStates::Invalid || _keyBuffer.size() > 9) {
        _state = TelexStates::Invalid;
        assert(CheckInvariants());
        return _state;
    }

    wchar_t c;
    auto ccase = FindLower(corig, &c);
    auto cat = ClassifyCharacter(c);
    if (cat == CharTypes::Uncategorized) {
        _state = TelexStates::Invalid;

    } else if (!_c1.size() && !_v.size() && (IS(cat, CharTypes::ConsoC1) || IS(cat, CharTypes::ConsoContinue))) {
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_c1.size() && !_v.size() && _c1 == L"g" && c == L'i') {
        // special treatment for 'gi'
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_c1 == L"d" && c == L'd' && ((_config.accept_separate_dd && _c1 == L"d") || (!_v.size() && !_c2.size()))) {
        // only used for 'dd'
        // relaxed constraint: !_v.size()
        _c1 = L"\x111";
        _respos.push_back(ResposTransitionC1);

    } else if (_c1 == L"\x111" && c == L'd') {
        // only used for 'dd'
        // relaxed constraint: !_v.size()
        if (_keyBuffer.size() > 1 && ToLower(_keyBuffer.rbegin()[1]) == L'd') {
            _keyBuffer.pop_back();
        }
        _state = TelexStates::Invalid;

    } else if (!_v.size() && !_c2.size() && _c1 != L"gi" && IS(cat, CharTypes::ConsoC1)) {
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (IS(cat, CharTypes::Vowel)) {
        // relaxed vowel position constraint: !_c2.size()
        // vowel parts (aeiouy)
        _v.push_back(c);
        auto before = _v.size();
        auto it = transitions.find(_v);
        if (it != transitions.end()) {
            _v = it->second;
            auto after = _v.size();
            // we don't yet take into account if _v grows in length due to the transition
            if (_keyBuffer.size() > 1 && _respos.back() == ResposTransitionV && c == ToLower(_keyBuffer.rbegin()[1])) {
                _keyBuffer.pop_back();
                if (after == before) {
                    // in case of double key, we want to pretend that the first V transition char didn't happen
                    // the rest is handled below
                    _respos.pop_back();
                }
            } else if (after < before) {
                _respos.push_back(ResposTransitionV);
            }
            if (after == before) {
                // in case of 'uơi' -> 'ươi', the transition char itself is a normal character
                // so it must be recorded as such rather than just a transition
                // the best solution however is to introduce flags into respos
                _cases.push_back(ccase);
                _respos.push_back(_respos_current++);
            }
        } else {
            // if there is no transition, there must be a new character -> must push case
            _cases.push_back(ccase);
            // invalidate if same char entered twice in a row in order to undo transition
            if (_keyBuffer.size() > 1 && _respos.back() == ResposTransitionV && c == ToLower(_keyBuffer.rbegin()[1])) {
                _keyBuffer.pop_back();
                _state = TelexStates::Invalid;
            }
            _respos.push_back(_respos_current++);
            if (_c2.size()) {
                // in case there exists no transition when _c2 is already typed
                // e.g. 'cace'
                _state = TelexStates::Invalid;
            }
        }

    } else if (_c1 == L"q" && _v.size() && IS(cat, CharTypes::VowelW)) {
        auto it = transitions_w_q.find(_v);
        if (it != transitions_w_q.end()) {
            _v = it->second;
            if (_c2.size()) {
                auto it2 = transitions_v_c2.find(_v);
                if (it2 != transitions_v_c2.end()) {
                    _v = it2->second;
                }
            }
            _respos.push_back(ResposTransitionW);
        } else {
            // pop back only if same char entered twice in a row
            if (c == ToLower(_keyBuffer.rbegin()[1])) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::Invalid;
        }
        // 'w' always keeps V size constant, don't push case

    } else if (_v.size() && IS(cat, CharTypes::VowelW)) {
        auto it = transitions_w.find(_v);
        if (it != transitions_w.end()) {
            _v = it->second;
            if (_c2.size()) {
                auto it2 = transitions_v_c2.find(_v);
                if (it2 != transitions_v_c2.end()) {
                    _v = it2->second;
                }
            }
            _respos.push_back(ResposTransitionW);
        } else {
            // pop back only if same char entered twice in a row
            if (c == ToLower(_keyBuffer.rbegin()[1])) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::Invalid;
        }
        // 'w' always keeps V size constant, don't push case

    } else if (!_c1.size() && !_v.size() && IS(cat, CharTypes::Tone) && IS(cat, CharTypes::Conso)) {
        // ambiguous (rsx) -> first character
        _c1.push_back(c);
        _cases.push_back(ccase);

    } else if ((_c1 == L"gi" || _v.size()) && IS(cat, CharTypes::Tone) && IS(cat, CharTypes::Conso)) {
        // ambiguous (rsx) -> tone
        auto newtone = GetTone(c);
        if (newtone != _t) {
            _t = newtone;
            _respos.push_back(ResposTone);
        } else {
            // the condition "_c1 == L"gi" || _v.size()" should ensure this already
            assert(_keyBuffer.length() > 1);
            // pop back only if same char entered twice in a row
            if (c == ToLower(_keyBuffer.rbegin()[1])) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::Invalid;
        }

    } else if (((_c1 == L"gi" && !_v.size()) || _v.size()) && !_c2.size() && IS(cat, CharTypes::ConsoC2)) {
        // word-ending consonants (cnpt)
        auto it = transitions_v_c2.find(_v);
        if (it != transitions_v_c2.end()) {
            _v = it->second;
        }
        _c2.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_c2.size() && IS(cat, CharTypes::ConsoContinue)) {
        // consonant continuation (dgh)
        _c2.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if ((_c1 == L"gi" || _v.size()) && IS(cat, CharTypes::Tone)) {
        // tones-only (fjz)
        // we must check for _c1 == 'gi' to allow typing 'gì'
        auto newtone = GetTone(c);
        if (newtone != _t) {
            _t = newtone;
            _respos.push_back(ResposTone);
        } else {
            assert(_keyBuffer.length() > 1);
            // pop back only if same char entered twice in a row
            if (c == ToLower(_keyBuffer.rbegin()[1])) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::Invalid;
        }

    } else if (cat == CharTypes::Shorthand) {
        // not implemented
        _state = TelexStates::Invalid;

    } else {
        _state = TelexStates::Invalid;
    }

    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Backspace() {
    std::wstring buf(_keyBuffer);

    if (_state == TelexStates::Invalid) {
        Reset();
        if (buf.size()) {
            buf.pop_back();
        }
        if (buf.size()) {
            _state = TelexStates::Invalid;
        }
        for (auto c : buf) {
            PushChar(c);
        }
        assert(CheckInvariants());
        return _state;
    } else if (_state != TelexStates::Valid) {
        return TelexStates::TxError;
    }

    // if cannot set tone like in Peek, treat this as invalid (but do not mark word as invalid for further correction)
    VInfo vinfo;
    auto found = GetTonePos(false, &vinfo);
    if (!found && _t != Tones::Z) {
        Reset();
        if (buf.size()) {
            buf.pop_back();
        }
        for (auto c : buf) {
            PushChar(c);
        }
        assert(CheckInvariants());
        return _state;
    }

    assert(_keyBuffer.size() == _respos.size());
    std::vector<int> rp(_respos);
    std::wstring oldc1(_c1);
    std::wstring oldv(_v);

    auto toDelete = static_cast<int>(_c1.size() + _v.size() + _c2.size()) - 1;

    Reset();

    // ensure only one key in the _keyBuffer is Tone
    int lastTone = -1;
    for (size_t i = 0; i < buf.size(); i++) {
        if (rp[i] == ResposTone) {
            lastTone = static_cast<int>(i);
            rp[i] = ResposExpunged;
        }
    }
    if (lastTone >= 0 && oldc1.size() + vinfo.tonepos < toDelete) {
        rp[lastTone] = ResposTone;
    }

    for (size_t i = 0; i < buf.size(); i++) {
        if (rp[i] >= toDelete) {
            // pass
        } else if (rp[i] == ResposExpunged) {
            // pass
        } else if (rp[i] == ResposTransitionC1 && toDelete == 0) {
            // assume that C1 transition is 'dd' on the first char
            // pass
        } else if (rp[i] == ResposTransitionV) {
            auto rp_it = respos.find(oldv);
            if (rp_it != respos.end() && (static_cast<int>(oldc1.size()) + rp_it->second) < toDelete) {
                PushChar(buf[i]);
            } else if (rp_it == respos.end()) {
                PushChar(buf[i]);
            }
        } else if (rp[i] == ResposTransitionW) {
            auto rpw_it = respos_w.find(oldv);
            if (rpw_it != respos_w.end() && (static_cast<int>(oldc1.size()) + rpw_it->second) < toDelete) {
                PushChar(buf[i]);
            } else if (rpw_it == respos_w.end()) {
                PushChar(buf[i]);
            }
        } else if (rp[i] == ResposTone) {
            VInfo vinfoRespos;
            // don't care about found or not
            GetTonePos(true, &vinfoRespos);
            if (vinfoRespos.tonepos >= 0 && vinfoRespos.tonepos < toDelete) {
                PushChar(buf[i]);
            }
        } else {
            PushChar(buf[i]);
        }
    }

    assert(CheckInvariants());
    return _state;
}

TelexStates TelexEngine::Commit() {
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid) {
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
    auto found = GetTonePos(false, &vinfo);
    if (!found) {
        _state = TelexStates::CommittedInvalid;
        return _state;
    }

    // routine changes buffers from this point

    if (vinfo.tonepos < 0 && _c1 == L"gi" && !_v.size()) {
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
    if (_state == TelexStates::Committed || _state == TelexStates::CommittedInvalid) {
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
    auto found = GetTonePos(false, &vinfo);
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
    _state = TelexStates::CommittedInvalid;
    return _state;
}

TelexStates TelexEngine::Backconvert(_In_ const std::wstring& s) {
    for (auto c : s) {
        if (c >= L'a' && c <= L'z') {
            PushChar(c);
        } else if (c >= L'A' && c <= L'Z') {
            PushChar(c);
        } else {
            auto clow = ToLower(c);
            auto it = backconversions.find(clow);
            if (c != clow) {
                // c is upper
                for (auto backc : it->second) {
                    PushChar(ToUpper(backc));
                }
            } else {
                for (auto backc : it->second) {
                    PushChar(backc);
                }
            }
        }
    }
    return _state;
}

TelexStates TelexEngine::GetState() const {
    return _state;
}

std::wstring TelexEngine::Retrieve() const {
    if (_state == TelexStates::Invalid || _state == TelexStates::CommittedInvalid) {
        return RetrieveRaw();
    }
    std::wstring result(_c1);
    result.append(_v);
    result.append(_c2);
    ApplyCases(result, _cases);
    return result;
}

std::wstring TelexEngine::RetrieveRaw() const {
    return std::wstring(_keyBuffer);
}

std::wstring TelexEngine::Peek() const {
    if (_state == TelexStates::Invalid) {
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
    if (vinfo.tonepos < 0 && _c1 == L"gi" && !_v.size()) {
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

std::wstring::size_type TelexEngine::Count() const {
    return _keyBuffer.size();
}

bool TelexEngine::FindTable(_Out_ map_iterator* it) const {
    if (_c1 == L"q") {
        *it = valid_v_q.find(_v);
        return *it != valid_v_q.end();
    } else if (_c1 == L"gi") {
        *it = valid_v_gi.find(_v);
        return *it != valid_v_gi.end();
    } else {
        if (!_config.oa_uy_tone1) {
            *it = valid_v_oa_uy.find(_v);
            if (*it != valid_v_oa_uy.end()) {
                return true;
            }
        }
        *it = valid_v.find(_v);
        return *it != valid_v.end();
    }
}

bool TelexEngine::GetTonePos(_In_ bool predict, _Out_ VInfo* vinfo) const {
    map_iterator it;
    auto found = FindTable(&it);
    VInfo retinfo = { 0 };
    if (found) {
        retinfo = it->second;
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
    return found;
}

bool TelexEngine::CheckInvariants() const {
    if (_state == TelexStates::TxError) {
        return false;
    }
    if (_c1.size() + _v.size() + _c2.size() > _keyBuffer.size()) {
        return false;
    }
    if (_state == TelexStates::Valid || _state == TelexStates::Committed) {
        if (_c1.size() + _v.size() + _c2.size() != _cases.size()) {
            return false;
        }
        if (_keyBuffer.size() != _respos.size()) {
            return false;
        }
        if (_respos.size() && _respos.back() > 0 && _respos.back() != _respos_current - 1) {
            return false;
        }
    }
    return true;
}

}
}
