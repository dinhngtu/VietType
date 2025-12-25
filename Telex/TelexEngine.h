// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <algorithm>
#include <optional>
#include <utility>
#include <vector>
#include <string>
#include "Telex.h"
#include "TelexMaps.h"

namespace VietType {
namespace Telex {

enum class Tones {
    Z,
    S,
    F,
    R,
    X,
    J,
};

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
    // transitions to Invalid:

    // removed by backspace (e.g. excessive tones), valid within Backspace only
    ResposExpunged = 0x80000000,
    // caused transition to Invalid through double key
    ResposDoubleUndo = 0x40000000,
    // caused transition to Invalid
    ResposInvalidate = 0x20000000,

    // transitions findable in valid words:

    // caused a C1 transition
    ResposTransitionC1 = 0x800000,
    // caused a V transition
    ResposTransitionV = 0x400000,
    // caused a W/WA transition
    ResposTransitionW = 0x200000,
    // is a tone
    ResposTone = 0x100000,

    // cached by autocorrect (e.g. the W in "nwuocs"), needs expunging/reordering at commit time
    ResposAutocorrect = 0x80000,
    // position inside final word corresponding to this respos
    ResposMask = 0xff,
    // mask of transitions findable in valid words
    ResposValidMask = 0xff0000,
};

enum class CharTypes : unsigned int {
    Uncategorized = 0,
    Vowel = 1 << 0,
    ConsoC1 = 1 << 1,
    ConsoC2 = 1 << 2,
    ConsoContinue = 1 << 3,
    Conso = ConsoC1 | ConsoC2 | ConsoContinue,
    W = 1 << 4,
    WA = 1 << 5,
    Dd = 1 << 6,
    Transition = 1 << 7,
    UW = 1 << 8,
    OW = 1 << 9,
    LeadingW = 1 << 10,
    // tones
    ToneZ = 1 << 16,
    ToneS = 1 << 17,
    ToneF = 1 << 18,
    ToneR = 1 << 19,
    ToneX = 1 << 20,
    ToneJ = 1 << 21,
    Tone = ToneZ | ToneS | ToneF | ToneR | ToneX | ToneJ,
};

constexpr CharTypes operator|(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<unsigned int>(lhs) | static_cast<unsigned int>(rhs));
}

constexpr CharTypes operator&(CharTypes lhs, CharTypes rhs) {
    return static_cast<CharTypes>(static_cast<unsigned int>(lhs) & static_cast<unsigned int>(rhs));
}

enum class TypingFlags : unsigned long long {
    Zero = 0,
    IsTelex = 0x1,
    NoAutocorrectLeadingW = 0x2,
    // legacy 1+
    NoAutocorrectLeadingEmptyW = 0x10, // telex only
    OptimizeEnDictionary = 0x20,
    Level1 = OptimizeEnDictionary,
    Level1Telex = Level1 | IsTelex | NoAutocorrectLeadingEmptyW,
    // legacy 2+
    OptimizeEnDictionary2 = 0x100,
    NoAutocorrectTrailingHWithoutTone = 0x200,
    NoAutocorrectTrailingG = 0x400,
    Level2 = Level1 | OptimizeEnDictionary2 | NoAutocorrectTrailingHWithoutTone | NoAutocorrectTrailingG,
    Level2Telex = Level2 | Level1Telex,
    // legacy 3+
    InvalidateOnVowelPostTone = 0x1000,
    InvalidateDoubleTone = 0x2000,
    Level3 = Level2 | InvalidateOnVowelPostTone | InvalidateDoubleTone,
    Level3Telex = Level3 | Level2Telex,
};

constexpr TypingFlags operator|(TypingFlags lhs, TypingFlags rhs) {
    return static_cast<TypingFlags>(static_cast<unsigned long long>(lhs) | static_cast<unsigned long long>(rhs));
}

constexpr TypingFlags operator&(TypingFlags lhs, TypingFlags rhs) {
    return static_cast<TypingFlags>(static_cast<unsigned long long>(lhs) & static_cast<unsigned long long>(rhs));
}

constexpr TypingFlags operator~(TypingFlags val) {
    return static_cast<TypingFlags>(~static_cast<unsigned long long>(val));
}

constexpr size_t NumOptimizationLevels = 8;

struct TypingStyle {
    CharTypes chartypes[128];
    ArrayMap<std::wstring_view, std::wstring_view, true> transitions;
    ArrayMap<wchar_t, std::wstring_view, true> backconversions;
    const std::wstring_view charlist;
    TypingFlags flags[NumOptimizationLevels];
    unsigned long max_optimize;
};

class TelexEngine : public ITelexEngine {
public:
    explicit TelexEngine(const TelexConfig& config);
    TelexEngine(const TelexEngine&) = delete;
    TelexEngine& operator=(const TelexEngine&) = delete;
    TelexEngine(TelexEngine&&) = default;
    TelexEngine& operator=(TelexEngine&&) = default;
    virtual ~TelexEngine() {
    }

    const TelexConfig& GetConfig() const override;
    void SetConfig(const TelexConfig& config) override;

    void Reset() override;
    TelexStates PushChar(wchar_t c) override;
    TelexStates Backspace() override;
    TelexStates Commit() override;
    TelexStates Cancel() override;
    TelexStates Backconvert(const std::wstring& s) override;

    constexpr TelexStates GetState() const override {
        return _state;
    }
    std::wstring Retrieve() const override;
    std::wstring RetrieveRaw() const override;
    std::wstring Peek() const override;
    constexpr std::wstring::size_type Count() const override {
        return _keyBuffer.size();
    }

    bool AcceptsChar(wchar_t c) const override;

    constexpr Tones GetTone() const {
        return _t;
    }
    constexpr const std::vector<unsigned int>& GetRespos() const {
        return _respos;
    }
    constexpr bool IsBackconverted() const {
        return _backconverted;
    }
    constexpr bool IsAutocorrected() const {
        return _autocorrected;
    }

    bool CheckInvariants() const;

private:
    struct TelexConfig _config;
    TypingFlags _cachedFlags;

    TelexStates _state = TelexStates::Valid;

    std::wstring _keyBuffer;
    std::wstring _c1;
    std::wstring _v;
    std::wstring _c2;
    Tones _t = Tones::Z;
    unsigned int _toneCount = 0;
    // don't use bool vector since that's special cased in the STL
    /// <summary>
    /// only use when valid;
    /// 1 = uppercase, 0 = lowercase
    /// </summary>
    std::vector<int> _cases;
    /// <summary>
    /// for each character in the _keyBuffer, record which output character it's responsible for,
    /// e.g. 'đuống' (dduoongs) _respos = 00122342 (T = tone, C = transition _c1, V = transition _v)
    ///                                    C  V  T
    /// note that respos position masks are only valid if state is Valid
    /// </summary>
    std::vector<unsigned int> _respos;
    unsigned int _respos_current = 0;
    bool _backconverted = false;
    bool _autocorrected = false;

private:
    bool CheckInvariantsBackspace(TelexStates prevState) const;

    template <typename T>
    bool TransitionV(const T& source, bool w_mode = false) {
        auto it = source.find(_v);
        if (it != source.end() &&
            (!w_mode || ((_v != it->second || _c2.empty()) && !(_respos.back() & ResposTransitionW)))) {
            _v = it->second;
            return true;
        } else {
            return false;
        }
    }

    const TypingStyle* GetTypingStyle() const;
    bool IsTypingStyle(TypingFlags flag) const {
        return static_cast<unsigned long long>(_cachedFlags & flag);
    }
    CharTypes ClassifyCharacter(_In_ wchar_t lc) const;
    void Invalidate();
    void InvalidateAndPopBack(wchar_t c);
    std::optional<std::pair<std::wstring_view, VInfo>> FindTable() const;
    bool GetTonePos(_In_ bool predict, _Out_ VInfo* vinfo) const;
    void ReapplyTone();
    bool HasValidRespos() const;
    void FeedNewResultChar(std::wstring& target, wchar_t c, bool ccase, unsigned int respos_flags = 0);
    TelexStates DoOptimizeAndAutocorrect();
};

} // namespace Telex
} // namespace VietType
