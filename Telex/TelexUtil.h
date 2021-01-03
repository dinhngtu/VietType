// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "TelexInternal.h"

namespace VietType {
namespace Telex {

enum class CharTypes : int {
    Uncategorized = 0,
    Commit = 1 << 0,
    ForceCommit = 1 << 1,
    Backspace = 1 << 2,
    Vowel = 1 << 3,
    VowelW = 1 << 4,
    // TODO: classification for vowel continue
    Conso = 1 << 5,
    ConsoC1 = 1 << 5 | 1 << 6,
    ConsoC2 = 1 << 5 | 1 << 7,
    ConsoContinue = 1 << 5 | 1 << 8,
    Tone = 1 << 9,
    Shorthand = 1 << 10,
};

using chartypes_t = std::underlying_type_t<CharTypes>;

constexpr inline CharTypes operator|(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<chartypes_t>(lhs) | static_cast<chartypes_t>(rhs));
}

constexpr inline CharTypes& operator|=(CharTypes lhs, CharTypes rhs) {
    lhs = static_cast<CharTypes>(static_cast<chartypes_t>(lhs) | static_cast<chartypes_t>(rhs));
    return lhs;
}

constexpr inline CharTypes operator&(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<chartypes_t>(lhs) & static_cast<chartypes_t>(rhs));
}

constexpr inline CharTypes& operator&=(CharTypes lhs, CharTypes rhs) {
    lhs = static_cast<CharTypes>(static_cast<chartypes_t>(lhs) & static_cast<chartypes_t>(rhs));
    return lhs;
}

/// <summary>classifies lower characters only</summary>
CharTypes ClassifyCharacter(_In_ wchar_t c);
Tones GetTone(_In_ wchar_t c);

}
}
