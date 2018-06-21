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

#include "TelexUtil.h"
#include "TelexData.h"

namespace VietType {
namespace Telex {

static CHR_CATEGORIES const letterClasses[26] = {
    CHR_CATEGORIES::VOWEL, // a
    CHR_CATEGORIES::CONSO_C1, // b
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_C2, // c
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_CONTINUE, // d
    CHR_CATEGORIES::VOWEL, // e
    CHR_CATEGORIES::TONE, // f
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_CONTINUE, // g
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_CONTINUE, // h
    CHR_CATEGORIES::VOWEL, // i
    CHR_CATEGORIES::TONE, // j
    CHR_CATEGORIES::CONSO_C1, // k
    CHR_CATEGORIES::CONSO_C1, // l
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_C2, // m
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_C2, // n
    CHR_CATEGORIES::VOWEL, // o
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_C2, // p
    CHR_CATEGORIES::CONSO_C1, // q
    CHR_CATEGORIES::TONE | CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_CONTINUE, // r
    CHR_CATEGORIES::TONE | CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_CONTINUE, // s
    CHR_CATEGORIES::CONSO_C1 | CHR_CATEGORIES::CONSO_C2, // t
    CHR_CATEGORIES::VOWEL, // u
    CHR_CATEGORIES::CONSO_C1, // v
    CHR_CATEGORIES::VOWEL_W, // w
    CHR_CATEGORIES::TONE | CHR_CATEGORIES::CONSO_C1, // x
    CHR_CATEGORIES::VOWEL, // y
    CHR_CATEGORIES::TONE, // z
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
}
