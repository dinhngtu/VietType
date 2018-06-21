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

#include "TelexData.h"

namespace VietType {
namespace Telex {

enum class CHR_CATEGORIES : int {
    UNCATEGORIZED = 0,
    COMMIT = 1 << 0,
    FORCECOMMIT = 1 << 1,
    BACKSPACE = 1 << 2,
    VOWEL = 1 << 3,
    VOWEL_W = 1 << 4,
    // TODO: classification for vowel continue
    CONSO = 1 << 5,
    CONSO_C1 = 1 << 5 | 1 << 6,
    CONSO_C2 = 1 << 5 | 1 << 7,
    CONSO_CONTINUE = 1 << 5 | 1 << 8,
    TONE = 1 << 9,
    SHORTHANDS = 1 << 10,
};

using CHR_CATEGORIES_T = std::underlying_type_t<CHR_CATEGORIES>;

inline CHR_CATEGORIES operator|(CHR_CATEGORIES lhs, CHR_CATEGORIES rhs) {
    return static_cast<CHR_CATEGORIES>(static_cast<CHR_CATEGORIES_T>(lhs) | static_cast<CHR_CATEGORIES_T>(rhs));
}

inline CHR_CATEGORIES& operator|=(CHR_CATEGORIES lhs, CHR_CATEGORIES rhs) {
    lhs = static_cast<CHR_CATEGORIES>(static_cast<CHR_CATEGORIES_T>(lhs) | static_cast<CHR_CATEGORIES_T>(rhs));
    return lhs;
}

inline CHR_CATEGORIES operator&(CHR_CATEGORIES lhs, CHR_CATEGORIES rhs) {
    return static_cast<CHR_CATEGORIES>(static_cast<CHR_CATEGORIES_T>(lhs) & static_cast<CHR_CATEGORIES_T>(rhs));
}

inline CHR_CATEGORIES& operator&=(CHR_CATEGORIES lhs, CHR_CATEGORIES rhs) {
    lhs = static_cast<CHR_CATEGORIES>(static_cast<CHR_CATEGORIES_T>(lhs) & static_cast<CHR_CATEGORIES_T>(rhs));
    return lhs;
}

/// <summary>classifies lower characters only</summary>
CHR_CATEGORIES ClassifyCharacter(wchar_t c);
TONES GetTone(wchar_t c);

}
}
