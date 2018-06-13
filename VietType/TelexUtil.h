#pragma once

#include "TelexData.h"

namespace Telex {
    enum class CHR_CATEGORIES {
        UNCATEGORIZED,
        COMMIT,
        FORCECOMMIT,
        BACKSPACE,

        VOWEL,
        VOWELW,
        WORDENDCONSO,
        OTHERCONSO,
        CONSOCONTINUE,
        TONES,
        TONECONSO,
        SHORTHANDS,
    };

    CHR_CATEGORIES ClassifyCharacter(wchar_t c);
    TONES GetTone(wchar_t c);
}
