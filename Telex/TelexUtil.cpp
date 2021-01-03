// Copyright (c) 2018 Dinh Ngoc Tu.
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

static const CharTypes letterClasses[26] = {
    CharTypes::Vowel, // a
    CharTypes::ConsoC1, // b
    CharTypes::ConsoC1 | CharTypes::ConsoC2, // c
    CharTypes::ConsoC1 | CharTypes::ConsoContinue, // d
    CharTypes::Vowel, // e
    CharTypes::Tone, // f
    CharTypes::ConsoC1 | CharTypes::ConsoContinue, // g
    CharTypes::ConsoC1 | CharTypes::ConsoContinue, // h
    CharTypes::Vowel, // i
    CharTypes::Tone, // j
    CharTypes::ConsoC1, // k
    CharTypes::ConsoC1, // l
    CharTypes::ConsoC1 | CharTypes::ConsoC2, // m
    CharTypes::ConsoC1 | CharTypes::ConsoC2, // n
    CharTypes::Vowel, // o
    CharTypes::ConsoC1 | CharTypes::ConsoC2, // p
    CharTypes::ConsoC1, // q
    CharTypes::Tone | CharTypes::ConsoC1 | CharTypes::ConsoContinue, // r
    CharTypes::Tone | CharTypes::ConsoC1 | CharTypes::ConsoContinue, // s
    CharTypes::ConsoC1 | CharTypes::ConsoC2, // t
    CharTypes::Vowel, // u
    CharTypes::ConsoC1, // v
    CharTypes::VowelW, // w
    CharTypes::Tone | CharTypes::ConsoC1, // x
    CharTypes::Vowel, // y
    CharTypes::Tone, // z
};

CharTypes ClassifyCharacter(_In_ wchar_t c) {
    if (c >= L'a' && c <= L'z') {
        return letterClasses[c - L'a'];
    }
    switch (c) {
    case L'\b':
        return CharTypes::Backspace;
    case L'\0':
        return CharTypes::ForceCommit;
    case L'\t':
    case L'\n':
    case L'\r':
    case L'|':
    case L'~':
        return CharTypes::Commit;
    case L'[':
    case L']':
    case L'{':
    case L'}':
        return CharTypes::Shorthand;
    }

    if (c >= 32 && c <= 64) {
        // ' ' to '@'
        return CharTypes::Commit;
    }
    if (c >= 91 && c <= 96) {
        return CharTypes::Commit;
    }

    return CharTypes::Uncategorized;
}

Tones GetTone(_In_ wchar_t c) {
    switch (c) {
    case L'z':
        return Tones::Z;
    case L'f':
        return Tones::F;
    case L'j':
        return Tones::J;
    case L'r':
        return Tones::R;
    case L's':
        return Tones::S;
    case L'x':
        return Tones::X;
    default:
        return Tones::Z;
    }
}

}
}
