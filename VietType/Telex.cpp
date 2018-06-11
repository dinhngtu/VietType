#include <cassert>

#include "Telex.h"
#include "TelexData.h"
#include "TelexUtil.h"

namespace Telex {
    TelexEngine::TelexEngine(TelexConfig config) {
        _config = config;
        reset();
    }

    TelexEngine::~TelexEngine() {
    }

    void TelexEngine::reset() {
        _state = TELEX_STATES::VALID;
        _keyBuffer.clear();
        _c1.clear();
        _v.clear();
        _c2.clear();
        _t = WORDTONES::Z;
    }

    TELEX_STATES TelexEngine::push_char(wchar_t c) {
        auto cat = ClassifyCharacter(towlower(c));

        if (cat != CHR_CATEGORIES::FORCECOMMIT) {
            _keyBuffer.push_back(c);
        }

        if (cat == CHR_CATEGORIES::COMMIT) {
            if (_state == TELEX_STATES::INVALID) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }

            if (_c1.size() == 1 && _c1[0] == L'q') {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }

            auto c1_it = valid_c1.find(_c1);
            if (c1_it == valid_c1.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }

            auto c2_it = valid_c2.find(_c2);
            if (c2_it == valid_c2.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }

            auto vpos_it = valid_v.find(_v);
            if (vpos_it == valid_v.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }
            wchar_t vatpos = _v[vpos_it->second];
            auto tonetable_it = transitions_tones.find(vatpos);
            // this should never happen
            assert(tonetable_it != transitions_tones.end());
            _v[vpos_it->second] = (tonetable_it->second)[(int)_t];

            _state = TELEX_STATES::COMMITTED;
            goto exit;

        } else if (cat == CHR_CATEGORIES::FORCECOMMIT) {
            if (_state == TELEX_STATES::INVALID) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }

            auto vpos_it = valid_v.find(_v);
            if (vpos_it == valid_v.end()) {
                _state = TELEX_STATES::COMMITTED_INVALID;
                goto exit;
            }
            wchar_t vatpos = _v[vpos_it->second];
            auto tonetable_it = transitions_tones.find(vatpos);
            // this should never happen
            assert(tonetable_it != transitions_tones.end());
            _v[vpos_it->second] = (tonetable_it->second)[(int)_t];

            _state = TELEX_STATES::COMMITTED;
            goto exit;

        } else if (cat == CHR_CATEGORIES::BACKSPACE) {
            if (_state != TELEX_STATES::VALID && _state != TELEX_STATES::INVALID) {
                return TELEX_STATES::ERROR;
            }
            // easy solution: remove the last character from the key buffer, then replay the rest
            auto tmp = _keyBuffer;
            tmp.pop_back();
            reset();
            for (const auto &rc : tmp) {
                push_char(rc);
            }

        } else if (_state == TELEX_STATES::INVALID || cat == CHR_CATEGORIES::UNCATEGORIZED) {
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

    std::wstring TelexEngine::retrieve() const {
        if (_state != TELEX_STATES::COMMITTED) {
            return std::wstring();
        }
        std::wstring result(_c1.begin(), _c1.end());
        result.append(_v.begin(), _v.end());
        result.append(_c2.begin(), _c2.end());
        result.push_back(_keyBuffer.back());
        return result;
    }

    std::wstring TelexEngine::retrieve_invalid() const {
        if (_state != TELEX_STATES::COMMITTED_INVALID) {
            return std::wstring();
        }
        return std::wstring(_keyBuffer.begin(), _keyBuffer.end());
    }

}
