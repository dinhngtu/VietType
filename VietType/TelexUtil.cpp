#include "TelexUtil.h"
#include "TelexData.h"

namespace Telex {
    static CHR_CATEGORIES const letterClasses[26] = {
        CHR_CATEGORIES::VOWEL, // a
        CHR_CATEGORIES::OTHERCONSO, // b
        CHR_CATEGORIES::WORDENDCONSO, // c
        CHR_CATEGORIES::CONSOCONTINUE, // d
        CHR_CATEGORIES::VOWEL, // e
        CHR_CATEGORIES::TONES, // f
        CHR_CATEGORIES::CONSOCONTINUE, // g
        CHR_CATEGORIES::CONSOCONTINUE, // h
        CHR_CATEGORIES::VOWEL, // i
        CHR_CATEGORIES::TONES, // j
        CHR_CATEGORIES::OTHERCONSO, // k
        CHR_CATEGORIES::OTHERCONSO, // l
        CHR_CATEGORIES::WORDENDCONSO, // m
        CHR_CATEGORIES::WORDENDCONSO, // n
        CHR_CATEGORIES::VOWEL, // o
        CHR_CATEGORIES::WORDENDCONSO, // p
        CHR_CATEGORIES::OTHERCONSO, // q
        CHR_CATEGORIES::TONECONSO, // r
        CHR_CATEGORIES::TONECONSO, // s
        CHR_CATEGORIES::WORDENDCONSO, // t
        CHR_CATEGORIES::VOWEL, // u
        CHR_CATEGORIES::OTHERCONSO, // v
        CHR_CATEGORIES::VOWELW, // w
        CHR_CATEGORIES::TONECONSO, // x
        CHR_CATEGORIES::VOWEL, // y
        CHR_CATEGORIES::TONES, // z
    };

    CHR_CATEGORIES ClassifyCharacter(wchar_t c) {
        if (c >= L'a' && c <= L'z') {
            return letterClasses[c - L'a'];
        }
        switch (c) {
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
