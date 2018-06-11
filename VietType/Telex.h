#pragma once

#include <vector>
#include <string>

#include "TelexData.h"
#include "TelexUtil.h"

#include "Export.h"

namespace Telex {
    // State transition is as follows:
    // VALID (initial) -> VALID|INVALID|COMMITTED (by pushing a commit character)
    // INVALID -> INVALID|COMMITTED_INVALID (by pushing a commit character)
    // COMMITTED -> VALID (reset)
    // COMMITTED_INVALID -> VALID (reset)

    enum class TELEX_STATES {
        VALID, // valid word, can accept more chars but cannot get result
        INVALID, // invalid word but can still accept more chars
        COMMITTED, // valid, tones have been applied, cannot accept any more chars (must get result or reset)
        COMMITTED_INVALID, // invalid, cannot accept any more chars
        ERROR = -1, // can never be a state, returned when push_char encounters an error
    };

    struct TelexConfig {
        bool oa_uy_tone1;
    };

    class TESTEXPORT TelexEngine {
    public:
        TelexEngine(struct TelexConfig config);
        ~TelexEngine();

        void reset();
        TELEX_STATES push_char(wchar_t c);

        std::wstring retrieve() const;
        std::wstring retrieve_invalid() const;

    private:
        struct TelexConfig _config;

        TELEX_STATES _state;

        std::vector<wchar_t> _keyBuffer;
        std::vector<wchar_t> _c1;
        std::vector<wchar_t> _v;
        std::vector<wchar_t> _c2;
        WORDTONES _t;
    };
}
