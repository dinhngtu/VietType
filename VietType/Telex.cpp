#include <cassert>

#include "Telex.h"
#include "TelexData.h"
#include "TelexUtil.h"
#include "Globals.h"

namespace Telex {
    TelexEngine::TelexEngine(_In_ TelexConfig config) {
        _config = config;
        _state = TELEX_STATES::VALID; // suppress warning
        _t = WORDTONES::Z; // suppress warning
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
        _t = WORDTONES::Z;
    }

    TELEX_STATES TelexEngine::PushChar(_In_ wchar_t c) {
        auto cat = ClassifyCharacter(towlower(c));

        _keyBuffer.push_back(c);

        if (_state == TELEX_STATES::INVALID || cat == CHR_CATEGORIES::UNCATEGORIZED) {
            _state = TELEX_STATES::INVALID;
            goto exit;

        } else if (!_c1.size() && !_v.size() && (cat == CHR_CATEGORIES::WORDENDCONSO || cat == CHR_CATEGORIES::OTHERCONSO || cat == CHR_CATEGORIES::CONSOCONTINUE)) {
            _c1.push_back(c);

        } else if (!_v.size() && !_c2.size() && (cat == CHR_CATEGORIES::CONSOCONTINUE)) {
            _c1.push_back(c);
            auto it = transitions.find(_c1);
            if (it != transitions.end()) {
                _c1 = it->second;
            }

        } else if (cat == CHR_CATEGORIES::VOWEL || cat == CHR_CATEGORIES::VOWELW) {
            // vowel parts (aeiouy)
            if (_c1.size() == 1 && _c1[0] == L'q' && _v.size() == 1 && _v[0] == L'u') {
                _c1.push_back('u');
                _v[0] = c;
            } else if (!_c2.size()) {
                _v.push_back(c);
                auto it = transitions.find(_v);
                if (it != transitions.end()) {
                    _v = it->second;
                }
            }

        } else if (cat == CHR_CATEGORIES::WORDENDCONSO) {
            // word-ending consonants(cnpt)
            auto it = transitions_v_c2.find(_v);
            if (it != transitions_v_c2.end()) {
                _v = it->second;
            }
            _c2.push_back(c);

        } else if (cat == CHR_CATEGORIES::CONSOCONTINUE) {
            // consonant continuation (dgh)
            _c2.push_back(c);

        } else if (_v.size() && cat == CHR_CATEGORIES::TONES) {
            // tones-only (fjz)
            _t = GetTone(c);

        } else if (!_c1.size() && cat == CHR_CATEGORIES::TONECONSO) {
            // ambiguous (rsx) -> first character
            _c1.push_back(c);

        } else if (_v.size() && cat == CHR_CATEGORIES::TONECONSO) {
            // ambiguous (rsx) -> tone
            _t = GetTone(c);

        } else if (cat == CHR_CATEGORIES::SHORTHANDS) {
            // not implemented
            _state = TELEX_STATES::INVALID;
            goto exit;

        } else {
            _state = TELEX_STATES::INVALID;
            goto exit;
        }

    exit:
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

        if (_c1.size() == 1 && _c1[0] == L'q') {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        auto c1_it = valid_c1.find(_c1);
        if (c1_it == valid_c1.end()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        auto c2_it = valid_c2.find(_c2);
        if (c2_it == valid_c2.end()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }

        auto vpos_it = valid_v.find(_v);
        if (vpos_it == valid_v.end()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }
        wchar_t vatpos = _v[vpos_it->second];
        auto tonetable_it = transitions_tones.find(vatpos);
        // this should never happen
        assert(tonetable_it != transitions_tones.end());
        _v[vpos_it->second] = (tonetable_it->second)[(int)_t];

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

        auto vpos_it = valid_v.find(_v);
        if (vpos_it == valid_v.end()) {
            _state = TELEX_STATES::COMMITTED_INVALID;
            return _state;
        }
        wchar_t vatpos = _v[vpos_it->second];
        auto tonetable_it = transitions_tones.find(vatpos);
        // this should never happen
        assert(tonetable_it != transitions_tones.end());
        _v[vpos_it->second] = (tonetable_it->second)[(int)_t];

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
        return result;
    }

    std::wstring TelexEngine::RetrieveInvalid() const {
        return std::wstring(_keyBuffer.begin(), _keyBuffer.end());
    }

}
