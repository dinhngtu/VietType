#include "TelexUtil.h"
#include "TelexData.h"

namespace Telex {
    CHR_CATEGORIES ClassifyCharacter(wchar_t c) {
        switch (c) {
        case L'w':
            return CHR_CATEGORIES::VOWELW;
        case L'\b':
            return CHR_CATEGORIES::BACKSPACE;
        case L'\0':
            return CHR_CATEGORIES::FORCECOMMIT;
        case L'\t':
        case L'\n':
        case L'\r':
        case L'|':
        case L'~':
            return CHR_CATEGORIES::COMMIT;
        case L'a':
        case L'e':
        case L'i':
        case L'o':
        case L'u':
        case L'y':
            return CHR_CATEGORIES::VOWEL;
        case L'c':
        case L'm':
        case L'n':
        case L'p':
        case L't':
            return CHR_CATEGORIES::WORDENDCONSO;
        case L'b':
        case L'k':
        case L'l':
        case L'q':
        case L'v':
            return CHR_CATEGORIES::OTHERCONSO;
        case L'd':
        case L'g':
        case L'h':
            return CHR_CATEGORIES::CONSOCONTINUE;
        case L'f':
        case L'j':
        case L'z':
            return CHR_CATEGORIES::TONES;
        case L'r':
        case L's':
        case L'x':
            return CHR_CATEGORIES::TONECONSO;
        case L'[':
        case L']':
        case L'{':
        case L'}':
            return CHR_CATEGORIES::SHORTHANDS;
        }

        if (c >= 32 && c <= 64) {
            // ' ' to '@'
            return CHR_CATEGORIES::COMMIT;
        }
        if (c >= 91 && c <= 96) {
            return CHR_CATEGORIES::COMMIT;
        }

        return CHR_CATEGORIES::UNCATEGORIZED;
    }

    TONES GetTone(wchar_t c) {
        switch (c) {
        case L'z':
            return TONES::Z;
        case L'f':
            return TONES::F;
        case L'j':
            return TONES::J;
        case L'r':
            return TONES::R;
        case L's':
            return TONES::S;
        case L'x':
            return TONES::X;
        default:
            return TONES::Z;
        }
    }
}
