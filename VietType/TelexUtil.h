#pragma once

#include "TelexData.h"

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
