// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#include "Common.h"
#include "Telex.h"

#define IS(cat, type) (static_cast<bool>((cat) & CHR_CATEGORIES::type))

namespace VietType {
namespace Telex {

enum RESPOS_TRANSITIONS {
    RESPOS_EXPUNGED = -1,
    RESPOS_TRANSITION_C1 = -2,
    RESPOS_TRANSITION_V = -3,
    RESPOS_TRANSITION_W = -4,
    RESPOS_TONE = -5,
};

static wchar_t ToUpper(wchar_t c) {
    // prechecking for these character ranges gives a sizable boost in performance
    if (c >= L'A' && c <= L'Z') {
        return c;
    }
    if (c >= L'a' && c <= L'z') {
        return c & ~32;
    }
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        return c & ~1;
    }
    auto it = touppermap.find(c);
    if (it != touppermap.end()) {
        return it->second;
    } else {
        return c;
    }
}

static wchar_t ToLower(wchar_t c) {
    if (c >= L'a' && c <= L'z') {
        return c;
    }
    if (c >= L'A' && c <= L'Z') {
        return c | 32;
    }
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        return c | 1;
    }
    auto it = tolowermap.find(c);
    if (it != tolowermap.end()) {
        return it->second;
    } else {
        return c;
    }
}

// return 1 if c is upper, 0 if c is lower
static int FindLower(wchar_t c, wchar_t *out) {
    if (c >= L'a' && c <= L'z') {
        *out = c;
        return 0;
    }
    if (c >= L'A' && c <= L'Z') {
        *out = c | 32;
        return 1;
    }
    if (c >= L'\x1ea0' && c <= L'\x1ef9') {
        int ret = !(c & 1);
        *out = c | 1;
        return ret;
    }
    auto it = tolowermap.find(c);
    if (it != tolowermap.end()) {
        *out = it->second;
        return 1;
    } else {
        *out = c;
        return 0;
    }
}

static wchar_t TranslateTone(wchar_t c, TONES t) {
    auto it = transitions_tones.find(c);
    // don't fail here since tone position prediction might give invalid v
    if (it != transitions_tones.end()) {
        return (it->second)[(int)t];
    } else {
        return c;
    }
}

/// <summary>destructive</summary>
static void ApplyCases(std::wstring& str, std::vector<int> const& cases) {
    assert(str.length() == cases.size());
    for (size_t i = 0; i < cases.size(); i++) {
        if (cases[i]) {
            str[i] = ToUpper(str[i]);
        }
    }
}

TelexEngine::TelexEngine(_In_ TelexConfig config) {
    _config = config;
    Reset();
}

TelexEngine::~TelexEngine() {
}

void TelexEngine::Reset() {
    DBG_DPRINT(L"%s", L"resetting engine");
    _state = TelexStates::VALID;
    _keyBuffer.clear();
    _c1.clear();
    _v.clear();
    _c2.clear();
    _t = TONES::Z;
    _cases.clear();
    _respos.clear();
    _respos_current = 0;
}

// remember to push into _cases when adding a new character
TelexStates TelexEngine::PushChar(_In_ wchar_t corig) {
    wchar_t c;
    auto ccase = FindLower(corig, &c);

    auto cat = ClassifyCharacter(c);

    _keyBuffer.push_back(corig);

    if (_state == TelexStates::INVALID || cat == CHR_CATEGORIES::UNCATEGORIZED) {
        _state = TelexStates::INVALID;

    } else if (!_c1.size() && !_v.size() && (IS(cat, CONSO_C1) || IS(cat, CONSO_CONTINUE))) {
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_c1.size() && !_v.size() && _c1 == L"g" && c == L'i') {
        // special treatment for 'gi'
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (!_v.size() && !_c2.size() && c == L'd') {
        // only used for 'dd'
        // relaxed constraint: !_v.size()
        _c1.push_back(c);
        auto before = _c1.size();
        auto it = transitions.find(_c1);
        if (it != transitions.end()) {
            _c1 = it->second;
            _respos.push_back(RESPOS_TRANSITION_C1);
        } else {
            _respos.push_back(_respos_current++);
        }
        auto after = _c1.size();
        // remember that c1 cannot grow when adding a char
        if (after == before) {
            _cases.push_back(ccase);
        }

    } else if (!_v.size() && !_c2.size() && _c1 != L"gi" && IS(cat, CONSO_C1)) {
        _c1.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (IS(cat, VOWEL)) {
        // relaxed vowel position constraint: !_c2.size()
        // vowel parts (aeiouy)
        _v.push_back(c);
        auto before = _v.size();
        auto it = transitions.find(_v);
        if (it != transitions.end()) {
            _v = it->second;
            auto after = _v.size();
            // we don't yet take into account if _v grows in length due to the transition
            if (after == before) {
                _cases.push_back(ccase);
            }
            _respos.push_back(RESPOS_TRANSITION_V);
        } else {
            // if there is no transition, there must be a new character -> must push case
            _cases.push_back(ccase);
            // invalidate if same char entered twice in a row in order to undo transition
            if (_keyBuffer.size() > 1 && c == _keyBuffer.rbegin()[1] && _respos.back() == RESPOS_TRANSITION_V) {
                _keyBuffer.pop_back();
                _state = TelexStates::INVALID;
            }
            _respos.push_back(_respos_current++);
            if (_c2.size()) {
                // in case there exists no transition when _c2 is already typed
                // fx. 'cace'
                _state = TelexStates::INVALID;
            }
        }

    } else if (_v.size() && IS(cat, VOWEL_W)) {
        auto it = transitions_w.find(_v);
        if (it != transitions_w.end()) {
            _v = it->second;
            if (_c2.size()) {
                auto it2 = transitions_v_c2.find(_v);
                if (it2 != transitions_v_c2.end()) {
                    _v = it2->second;
                }
                _respos.push_back(RESPOS_TRANSITION_W);
            }
        } else {
            // pop back only if same char entered twice in a row
            if (c == _keyBuffer.rbegin()[1]) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::INVALID;
        }
        // 'w' always keeps V size constant, don't push case

    } else if (!_c1.size() && !_v.size() && IS(cat, TONE) && IS(cat, CONSO)) {
        // ambiguous (rsx) -> first character
        _c1.push_back(c);
        _cases.push_back(ccase);

    } else if ((_c1 == L"gi" || _v.size()) && IS(cat, TONE) && IS(cat, CONSO)) {
        // ambiguous (rsx) -> tone
        auto newtone = GetTone(c);
        if (newtone != _t) {
            _t = newtone;
            _respos.push_back(RESPOS_TONE);
        } else {
            // the condition "_c1 == L"gi" || _v.size()" should ensure this already
            assert(_keyBuffer.length() > 1);
            // pop back only if same char entered twice in a row
            if (c == _keyBuffer.rbegin()[1]) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::INVALID;
        }

    } else if (((_c1 == L"gi" && !_v.size()) || _v.size()) && !_c2.size() && IS(cat, CONSO_C2)) {
        // word-ending consonants (cnpt)
        auto it = transitions_v_c2.find(_v);
        if (it != transitions_v_c2.end()) {
            _v = it->second;
        }
        _c2.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if (_c2.size() && IS(cat, CONSO_CONTINUE)) {
        // consonant continuation (dgh)
        _c2.push_back(c);
        _cases.push_back(ccase);
        _respos.push_back(_respos_current++);

    } else if ((_c1 == L"gi" || _v.size()) && IS(cat, TONE)) {
        // tones-only (fjz)
        // we must check for _c1 == 'gi' to allow typing 'gì'
        auto newtone = GetTone(c);
        if (newtone != _t) {
            _t = newtone;
            _respos.push_back(RESPOS_TONE);
        } else {
            assert(_keyBuffer.length() > 1);
            // pop back only if same char entered twice in a row
            if (c == _keyBuffer.rbegin()[1]) {
                _keyBuffer.pop_back();
            }
            _state = TelexStates::INVALID;
        }

    } else if (cat == CHR_CATEGORIES::SHORTHANDS) {
        // not implemented
        _state = TelexStates::INVALID;

    } else {
        _state = TelexStates::INVALID;
    }

    return _state;
}

TelexStates TelexEngine::Backspace() {
    auto buf = _keyBuffer;

    if (_state == TelexStates::INVALID) {
        Reset();
        if (buf.size()) {
            buf.pop_back();
        }
        if (buf.size()) {
            _state = TelexStates::INVALID;
        }
        for (auto c : buf) {
            PushChar(c);
        }
        return _state;
    } else if (_state != TelexStates::VALID) {
        return TelexStates::TXERROR;
    }

    assert(_keyBuffer.size() == _respos.size());
    auto rp = _respos;
    auto oldc1 = _c1;
    auto oldv = _v;

    auto toDelete = static_cast<int>(_c1.size() + _v.size() + _c2.size()) - 1;

    Reset();

    // ensure only one key in the _keyBuffer is TONE
    int lastTone = -1;
    for (size_t i = 0; i < buf.size(); i++) {
        if (rp[i] == RESPOS_TONE) {
            lastTone = static_cast<int>(i);
            rp[i] = RESPOS_EXPUNGED;
        }
    }
    if (lastTone >= 0) {
        rp[lastTone] = RESPOS_TONE;
    }

    for (size_t i = 0; i < buf.size(); i++) {
        if (rp[i] >= toDelete) {
            // pass
        } else if (rp[i] == RESPOS_EXPUNGED) {
            // pass
        } else if (rp[i] == RESPOS_TRANSITION_C1 && toDelete == 0) {
            // assume that C1 transition is 'dd' on the first char
            // pass
        } else if (rp[i] == RESPOS_TRANSITION_V) {
            auto rp_it = respos.find(oldv);
            if (rp_it != respos.end() && (static_cast<int>(oldc1.size()) + rp_it->second) < toDelete) {
                PushChar(buf[i]);
            }
        } else if (rp[i] == RESPOS_TRANSITION_W) {
            auto rpw_it = respos_w.find(oldv);
            if (rpw_it != respos_w.end() && (static_cast<int>(oldc1.size()) + rpw_it->second) < toDelete) {
                PushChar(buf[i]);
            }
        } else if (rp[i] == RESPOS_TONE) {
            VINFO vinfo;
            // don't care about found or not
            GetTonePos(true, &vinfo);
            if (vinfo.tonepos >= 0 && vinfo.tonepos < toDelete) {
                PushChar(buf[i]);
            }
        } else {
            PushChar(buf[i]);
        }
    }
    return _state;
}

TelexStates TelexEngine::Commit() {
    if (_state == TelexStates::COMMITTED || _state == TelexStates::COMMITTED_INVALID) {
        return _state;
    }

    if (_state == TelexStates::INVALID) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }

    if (!_keyBuffer.size()) {
        _state = TelexStates::COMMITTED;
        return _state;
    }

    // validate c1
    auto c1_it = valid_c1.find(_c1);
    if (c1_it == valid_c1.end()) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }

    // validate c2
    auto c2_it = valid_c2.find(_c2);
    if (c2_it == valid_c2.end()) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }
    if (c2_it->second && !(_t == TONES::S || _t == TONES::J)) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }

    // validate v and get tone position
    VINFO vinfo;
    auto found = GetTonePos(false, &vinfo);
    if (!found) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }

    // routine changes buffers from this point

    if (vinfo.tonepos < 0 && _c1 == L"gi" && !_v.size()) {
        // fixup 'gi' by moving 'i' to _v
        _c1.pop_back();
        _v.push_back(L'i');
        vinfo.tonepos = 0;
    } else if (vinfo.c2mode == C2MODE::MUSTC2 && !_c2.size()) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    } else if (vinfo.c2mode == C2MODE::NOC2 && _c2.size()) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }

    _v[vinfo.tonepos] = TranslateTone(_v[vinfo.tonepos], _t);

    _state = TelexStates::COMMITTED;
    return _state;
}

TelexStates TelexEngine::ForceCommit() {
    if (_state == TelexStates::COMMITTED || _state == TelexStates::COMMITTED_INVALID) {
        return _state;
    }

    if (_state == TelexStates::INVALID) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }

    if (!_keyBuffer.size()) {
        _state = TelexStates::COMMITTED;
        return _state;
    }

    VINFO vinfo;
    auto found = GetTonePos(false, &vinfo);
    if (!found) {
        _state = TelexStates::COMMITTED_INVALID;
        return _state;
    }
    _v[vinfo.tonepos] = TranslateTone(_v[vinfo.tonepos], _t);

    _state = TelexStates::COMMITTED;
    return _state;
}

TelexStates TelexEngine::Cancel() {
    _state = TelexStates::COMMITTED_INVALID;
    return _state;
}

TelexStates TelexEngine::Backconvert(std::wstring const& s) {
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
    if (_state == TelexStates::INVALID || _state == TelexStates::COMMITTED_INVALID) {
        DBG_DPRINT(L"invalid retrieve call state %d", _state);
        return std::wstring();
    }
    std::wstring result(_c1);
    result.append(_v);
    result.append(_c2);
    ApplyCases(result, _cases);
    return result;
}

std::wstring TelexEngine::RetrieveInvalid() const {
    return std::wstring(_keyBuffer);
}

std::wstring TelexEngine::Peek() const {
    if (_state == TelexStates::INVALID) {
        return RetrieveInvalid();
    }

    std::wstring result(_c1);
    result.append(_v);

    VINFO vinfo;
    auto found = GetTonePos(true, &vinfo);
    if (!found) {
        result.append(_c2);
        ApplyCases(result, _cases);
        return result;
    }

    // fixup 'gi'
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

bool TelexEngine::FindTable(_Out_ map_iterator * it) const {
    if (_c1 == L"q") {
        *it = valid_v_q.find(_v);
        return *it != valid_v_q.end();
    } else if (_c1 == L"gi") {
        *it = valid_v_gi.find(_v);
        return *it != valid_v_gi.end();
    } else {
        *it = valid_v.find(_v);
        return *it != valid_v.end();
    }
}

bool TelexEngine::GetTonePos(bool predict, Telex::VINFO *vinfo) const {
    map_iterator it;
    auto found = FindTable(&it);
    VINFO retinfo = { 0 };
    if (found) {
        retinfo = it->second;
    } else if (predict) {
        // guess tone position if _v is not known
        switch (_v.size()) {
        case 1:
            retinfo.tonepos = 0;
            retinfo.c2mode = C2MODE::EITHER;
            break;
        case 2:
        case 3:
            retinfo.tonepos = 1;
            retinfo.c2mode = C2MODE::EITHER;
            break;
        default:
            retinfo.tonepos = -1;
            retinfo.c2mode = C2MODE::EITHER;
            break;
        }
    }
    *vinfo = retinfo;
    return found;
}

}
}
