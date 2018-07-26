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

inline CharTypes operator|(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<chartypes_t>(lhs) | static_cast<chartypes_t>(rhs));
}

inline CharTypes& operator|=(CharTypes lhs, CharTypes rhs) {
    lhs = static_cast<CharTypes>(static_cast<chartypes_t>(lhs) | static_cast<chartypes_t>(rhs));
    return lhs;
}

inline CharTypes operator&(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<chartypes_t>(lhs) & static_cast<chartypes_t>(rhs));
}

inline CharTypes& operator&=(CharTypes lhs, CharTypes rhs) {
    lhs = static_cast<CharTypes>(static_cast<chartypes_t>(lhs) & static_cast<chartypes_t>(rhs));
    return lhs;
}

// classifies lower characters only
CharTypes ClassifyCharacter(_In_ wchar_t c);
Tones GetTone(_In_ wchar_t c);

}
}
