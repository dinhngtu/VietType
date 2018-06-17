#include <cassert>

#include "Telex.h"
#include "TelexData.h"
#include "TelexUtil.h"
#include "Globals.h"

namespace Telex {
    static wchar_t ToUpper(wchar_t c) {
        auto it = touppermap.find(c);
        if (it != touppermap.end()) {
            return it->second;
        } else {
            return c;
        }
    }

    static int FindLower(wchar_t c, wchar_t *out) {
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
        _state = TELEX_STATES::VALID; // suppress warning
        _t = TONES::Z; // suppress warning
        Reset();
    }

    TelexEngine::~TelexEngine() {
    }

    void TelexEngine::Reset() {
        _state = TELEX_STATES::VALID;
        _keyBuffer.clear();
        _c1.clear();
        _v.clear();
        _c2.clear();
        _t = TONES::Z;
        _cases.clear();
    }

    // remember to push into _cases when adding a new character
    TELEX_STATES TelexEngine::PushChar(_In_ wchar_t corig) {
        wchar_t c;
        auto ccase = FindLower(corig, &c);

        auto cat = ClassifyCharacter(c);

        _keyBuffer.push_back(corig);

        if (_state == TELEX_STATES::INVALID || cat == CHR_CATEGORIES::UNCATEGORIZED) {
            _state = TELEX_STATES::INVALID;

        } else if (!_c1.size() && !_v.size() && (cat == CHR_CATEGORIES::WORDENDCONSO || cat == CHR_CATEGORIES::OTHERCONSO || cat == CHR_CATEGORIES::CONSOCONTINUE)) {
            _c1.push_back(c);
            _cases.push_back(ccase);

        } else if (_c1.size() && !_v.size() && _c1 == L"g" && c == L'i') {
            // special treatment for 'gi'
            _c1.push_back(c);
            _cases.push_back(ccase);

        } else if (!_c2.size() && c == L'd') {
            // only used for 'dd'
            // relaxed constraint: !_v.size()
            _c1.push_back(c);
            auto before = _c1.size();
            auto it = transitions.find(_c1);
            if (it != transitions.end()) {
                _c1 = it->second;
            }
            auto after = _c1.size();
            // remember that c1 cannot grow when adding a char
            if (after == before) {
                _cases.push_back(ccase);
            }

        } else if (!_v.size() && !_c2.size() && cat == CHR_CATEGORIES::CONSOCONTINUE) {
            _c1.push_back(c);
            _cases.push_back(ccase);

        } else if (cat == CHR_CATEGORIES::VOWEL) {
            // relaxed vowel position constraint: !_c2.size()
            // vowel parts (aeiouy)
            _v.push_back(c);
            auto before = _v.size();
            auto it = transitions.find(_v);
            if (it != transitions.end()) {
                _v = it->second;
            }
            auto after = _v.size();
            if (after == before) {
                _cases.push_back(ccase);
            }

        } else if (_v.size() && cat == CHR_CATEGORIES::VOWELW) {
            auto it = transitions_w.find(_v);
            if (it != transitions_w.end()) {
                _v = it->second;
                if (_c2.size()) {
                    auto it2 = transitions_v_c2.find(_v);
                    if (it2 != transitions_v_c2.end()) {
                        _v = it2->second;
                    }
                }
            } else {
                _state = TELEX_STATES::INVALID;
            }
            // 'w' always keeps V size constant, don't push case

        } else if (cat == CHR_CATEGORIES::WORDENDCONSO) {
            // word-ending consonants (cnpt)
            auto it = transitions_v_c2.find(_v);
            if (it != transitions_v_c2.end()) {
                _v = it->second;
            }
            _c2.push_back(c);
            _cases.push_back(ccase);

        } else if (_c2.size() && cat == CHR_CATEGORIES::CONSOCONTINUE) {
            // consonant continuation (dgh)
            _c2.push_back(c);
            _cases.push_back(ccase);

        } else if ((_c1 == L"gi" || _v.size()) && cat == CHR_CATEGORIES::TONES) {
            // tones-only (fjz)
            auto newtone = GetTone(c);
            if (newtone != _t) {
                _t = newtone;
            } else {
                _t = TONES::Z;
                _state = TELEX_STATES::INVALID;
            }

        } else if (!_c1.size() && !_v.size() && cat == CHR_CATEGORIES::TONECONSO) {
            // ambiguous (rsx) -> first character
            _c1.push_back(c);
            _cases.push_back(ccase);

        } else if ((_c1 == L"gi" || _v.size()) && cat == CHR_CATEGORIES::TONECONSO) {
            // ambiguous (rsx) -> tone
            auto newtone = GetTone(c);
            if (newtone != _t) {
                _t = newtone;
            } else {
                _t = TONES::Z;
                _state = TELEX_STATES::INVALID;
            }

        } else if (cat == CHR_CATEGORIES::SHORTHANDS) {
            // not implemented
            _state = TELEX_STATES::INVALID;

        } else {
            _state = TELEX_STATES::INVALID;
        }

        return _state;
    }

    TELEX_STATES TelexEngine::Backspace() {
        auto buf = _keyBuffer;
        if (buf.size()) {
            buf.pop_back();
        }
        Reset();
        for (auto c : buf) {
            PushChar(c);
        }
        return _state;
    }

    TELEX_STATES TelexEngine::Commit() {
        if (_state == TELEX_STATES::COMMITTED || _state == TELEX_STATES::COMMITTED_INVALID) {
            return _state;
        }

        if (_state == TELEX_STATES::INVALID) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        // validate c1
        auto c1_it = valid_c1.find(_c1);
        if (c1_it == valid_c1.end()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        // validate c2
        auto c2_it = valid_c2.find(_c2);
        if (c2_it == valid_c2.end()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }
        if (c2_it->second && !(_t == TONES::S || _t == TONES::J)) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        // validate v and get tone position
        map_iterator it;
        auto found = FindTable(&it);
        if (!found) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }
        auto tonepos = it->second.tonepos;

        // routine changes buffers from this point

        if (tonepos < 0 && _c1 == L"gi" && !_v.size()) {
            // fixup 'gi'
            _c1.pop_back();
            _v.push_back(L'i');
            tonepos = 0;
        } else if (it->second.c2mode == C2MODE::MUSTC2 && !_c2.size()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        } else if (it->second.c2mode == C2MODE::NOC2 && _c2.size()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        _v[tonepos] = TranslateTone(_v[tonepos], _t);

        _state = TELEX_STATES::COMMITTED;
        return _state;
    }

    TELEX_STATES TelexEngine::ForceCommit() {
        if (_state == TELEX_STATES::COMMITTED || _state == TELEX_STATES::COMMITTED_INVALID) {
            return _state;
        }

        if (_state == TELEX_STATES::INVALID) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        map_iterator it;
        auto found = FindTable(&it);
        if (!found) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }
        _v[it->second.tonepos] = TranslateTone(_v[it->second.tonepos], _t);

        _state = TELEX_STATES::COMMITTED;
        return _state;
    }

    TELEX_STATES TelexEngine::Cancel() {
        _state = TELEX_STATES::COMMITTED_INVALID;
        return _state;
    }

    std::wstring TelexEngine::Retrieve() const {
        if (_state == TELEX_STATES::INVALID || _state == TELEX_STATES::COMMITTED_INVALID) {
            //throw std::exception("invalid retrieval call state");
            DBGPRINT(L"invalid retrieve call state %d", _state);
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
        std::wstring result(_c1);
        result.append(_v);

        int tonepos;
        map_iterator it;
        auto found = FindTable(&it);
        // guess tone position if V is not known
        if (found) {
            tonepos = it->second.tonepos;
        } else {
            switch (_v.size()) {
            case 1:
                tonepos = 0;
                break;
            case 2:
            case 3:
                tonepos = 1;
                break;
            default:
                tonepos = -1;
            }
        }

        // fixup 'gi'
        if (tonepos < 0 && _c1 == L"gi" && !_v.size()) {
            tonepos = (int)_c1.size() - 1;
            wchar_t vatpos = TranslateTone(_c1[tonepos], _t);
            result[tonepos] = vatpos;
        } else if (tonepos >= 0) {
            wchar_t vatpos = TranslateTone(_v[tonepos], _t);
            result[_c1.size() + tonepos] = vatpos;
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
}
