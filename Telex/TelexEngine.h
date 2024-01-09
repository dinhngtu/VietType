#pragma once

namespace VietType {
namespace Telex {

enum class C2Mode {
    Either,
    MustC2,
    NoC2,
};

struct VInfo {
    int tonepos;
    C2Mode c2mode;
};

enum ResposTransitions {
    ResposExpunged = 0x40000000,
    ResposTransitionC1 = 0x20000000,
    ResposTransitionV = 0x10000000,
    ResposTransitionW = 0x8000000,
    ResposTone = 0x4000000,
    ResposDoubleUndo = 0x2000000,
    ResposInvalidate = 0x1000000,
    //
    ResposMask = 0xff,
};

enum class CharTypes : unsigned int {
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

constexpr CharTypes operator|(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

constexpr CharTypes operator&(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

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

} // namespace Telex
} // namespace VietType
