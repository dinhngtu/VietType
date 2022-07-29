// SPDX-License-Identifier: GPL-3.0-only

#include "TelexUtil.h"
#include "TelexData.h"

namespace VietType {
namespace Telex {

static const CharTypes letterClasses[26] = {
    CharTypes::Vowel,                                                // a
    CharTypes::ConsoC1,                                              // b
    CharTypes::ConsoC1 | CharTypes::ConsoC2,                         // c
    CharTypes::ConsoC1 | CharTypes::ConsoContinue,                   // d
    CharTypes::Vowel,                                                // e
    CharTypes::Tone,                                                 // f
    CharTypes::ConsoC1 | CharTypes::ConsoContinue,                   // g
    CharTypes::ConsoC1 | CharTypes::ConsoContinue,                   // h
    CharTypes::Vowel,                                                // i
    CharTypes::Tone,                                                 // j
    CharTypes::ConsoC1,                                              // k
    CharTypes::ConsoC1,                                              // l
    CharTypes::ConsoC1 | CharTypes::ConsoC2,                         // m
    CharTypes::ConsoC1 | CharTypes::ConsoC2,                         // n
    CharTypes::Vowel,                                                // o
    CharTypes::ConsoC1 | CharTypes::ConsoC2,                         // p
    CharTypes::ConsoC1,                                              // q
    CharTypes::Tone | CharTypes::ConsoC1 | CharTypes::ConsoContinue, // r
    CharTypes::Tone | CharTypes::ConsoC1 | CharTypes::ConsoContinue, // s
    CharTypes::ConsoC1 | CharTypes::ConsoC2,                         // t
    CharTypes::Vowel,                                                // u
    CharTypes::ConsoC1,                                              // v
    CharTypes::VowelW,                                               // w
    CharTypes::Tone | CharTypes::ConsoC1,                            // x
    CharTypes::Vowel,                                                // y
    CharTypes::Tone,                                                 // z
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

} // namespace Telex
} // namespace VietType
