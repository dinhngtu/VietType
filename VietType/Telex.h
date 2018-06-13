#pragma once

#include <vector>
#include <string>

#include "TelexData.h"
#include "TelexUtil.h"

#include "Export.h"

namespace Telex {
    // State transition is as follows:

    // VALID (initial) -> VALID|INVALID (by pushing a character)
    // INVALID -> INVALID (by pushing a character)

    // VALID -> COMMITTED|COMMITTED_INVALID (by calling Commit/ForceCommit)
    // INVALID -> COMMITTED_INVALID (by calling Commit/ForceCommit)
    // VALID/INVALID -> COMMITTED_INVALID (by calling Cancel)

    // COMMITTED: can call Retrieve/RetrieveInvalid, does not change state
    // COMMITTED_INVALID: can call RetrieveInvalid, does not change state

    // COMMITTED|COMMITTED_INVALID -> VALID (Reset)

    enum class TELEX_STATES {
        VALID, // valid word, can accept more chars but cannot get result
        INVALID, // invalid word but can still accept more chars
        COMMITTED, // valid, tones have been applied, cannot accept any more chars (must get result or Reset)
        COMMITTED_INVALID, // invalid, cannot accept any more chars
        TXERROR = -1, // can never be a state, returned when PushChar encounters an error
    };

    struct TelexConfig {
        bool oa_uy_tone1;
    };

    class TESTEXPORT TelexEngine {
    public:
        TelexEngine(_In_ struct TelexConfig config);
        ~TelexEngine();

        void Reset();
        TELEX_STATES PushChar(_In_ wchar_t c);

        TELEX_STATES Commit();
        TELEX_STATES ForceCommit();
        TELEX_STATES Cancel();

        std::wstring Retrieve() const;
        std::wstring RetrieveInvalid() const;
        std::wstring Peek() const;

    private:
        struct TelexConfig _config;

        TELEX_STATES _state;

        std::vector<wchar_t> _keyBuffer;
        std::vector<wchar_t> _c1;
        std::vector<wchar_t> _v;
        std::vector<wchar_t> _c2;
        TONES _t;
    };
}
