#include <cassert>
#include <locale>

#include "Telex.h"
#include "TelexData.h"
#include "TelexUtil.h"
#include "Globals.h"

namespace Telex {
    // not sure if using the preferred locale will break upper/lower
    static std::locale const internal_locale("");

    static wchar_t TranslateTone(wchar_t c, TONES t) {
        auto it = transitions_tones.find(c);
        assert(it != transitions_tones.end());
        return (it->second)[(int)t];
    }

    /// <summary>destructive</summary>
    static void ApplyCases(std::wstring& str, std::vector<int> const& cases) {
        assert(str.length() == cases.size());
        for (int i = 0; i < cases.size(); i++) {
            if (cases[i]) {
                str[i] = std::toupper(str[i], internal_locale);
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
        auto ccase = std::isupper(corig, internal_locale);
        auto c = std::tolower(corig, internal_locale);
        auto cat = ClassifyCharacter(c);

        _keyBuffer.push_back(c);

        if (_state == TELEX_STATES::INVALID || cat == CHR_CATEGORIES::UNCATEGORIZED) {
            _state = TELEX_STATES::INVALID;

        } else if (!_c1.size() && !_v.size() && (cat == CHR_CATEGORIES::WORDENDCONSO || cat == CHR_CATEGORIES::OTHERCONSO || cat == CHR_CATEGORIES::CONSOCONTINUE)) {
            _c1.push_back(c);
            _cases.push_back(ccase);

        } else if (!_v.size() && !_c2.size() && (cat == CHR_CATEGORIES::CONSOCONTINUE)) {
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

        } else if (cat == CHR_CATEGORIES::VOWEL || cat == CHR_CATEGORIES::VOWELW) {
            // vowel parts (aeiouy)
            if (!_c2.size()) {
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
            }

        } else if (cat == CHR_CATEGORIES::WORDENDCONSO) {
            // word-ending consonants(cnpt)
            auto it = transitions_v_c2.find(_v);
            if (it != transitions_v_c2.end()) {
                _v = it->second;
            }
            _c2.push_back(c);
            _cases.push_back(ccase);

        } else if (cat == CHR_CATEGORIES::CONSOCONTINUE) {
            // consonant continuation (dgh)
            _c2.push_back(c);
            _cases.push_back(ccase);

        } else if (_v.size() && cat == CHR_CATEGORIES::TONES) {
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

        } else if (_v.size() && cat == CHR_CATEGORIES::TONECONSO) {
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
        std::map<std::vector<wchar_t>, VINFO>::const_iterator vpos_it;
        if (_c1.size() == 1 && _c1[0] == L'q') {
            vpos_it = valid_v_q.find(_v);
            if (vpos_it == valid_v_q.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                return _state;
            }
        } else {
            vpos_it = valid_v.find(_v);
            if (vpos_it == valid_v.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                return _state;
            }
        }

        if (vpos_it->second.c2mode == C2MODE::MUSTC2 && !_c2.size()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        } else if (vpos_it->second.c2mode == C2MODE::NOC2 && _c2.size()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        // routine changes buffers from this point

        _v[vpos_it->second.tonepos] = TranslateTone(_v[vpos_it->second.tonepos], _t);

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

        std::map<std::vector<wchar_t>, VINFO>::const_iterator vpos_it;
        if (_c1.size() == 1 && _c1[0] == L'q') {
            vpos_it = valid_v_q.find(_v);
            if (vpos_it == valid_v_q.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                return _state;
            }
        } else {
            vpos_it = valid_v.find(_v);
            if (vpos_it == valid_v.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                return _state;
            }
        }

        _v[vpos_it->second.tonepos] = TranslateTone(_v[vpos_it->second.tonepos], _t);

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
        std::wstring result(_c1.begin(), _c1.end());
        result.append(_v.begin(), _v.end());
        result.append(_c2.begin(), _c2.end());
        ApplyCases(result, _cases);
        return result;
    }

    std::wstring TelexEngine::RetrieveInvalid() const {
        return std::wstring(_keyBuffer.begin(), _keyBuffer.end());
    }

    std::wstring TelexEngine::Peek() const {
        int tonepos;

        // validate v and get tone position
        std::map<std::vector<wchar_t>, VINFO>::const_iterator vpos_it;
        bool vpos_found;
        if (_c1.size() == 1 && _c1[0] == L'q') {
            vpos_it = valid_v_q.find(_v);
            vpos_found = vpos_it != valid_v_q.end();
        } else {
            vpos_it = valid_v.find(_v);
            vpos_found = vpos_it != valid_v.end();
        }

        // guess tone position if V is not known
        if (vpos_found) {
            tonepos = vpos_it->second.tonepos;
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
                return RetrieveInvalid();
            }
        }

        wchar_t vatpos = TranslateTone(_v[tonepos], _t);

        std::wstring result(_c1.begin(), _c1.end());
        result.append(_v.begin(), _v.end());
        result[_c1.size() + tonepos] = vatpos;
        result.append(_c2.begin(), _c2.end());
        ApplyCases(result, _cases);

        return result;
    }
}
