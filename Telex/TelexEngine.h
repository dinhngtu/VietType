// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <optional>
#include <utility>
#include <vector>
#include <string>

namespace VietType {
namespace Telex {

enum class Tones {
    Z,
    F,
    J,
    R,
    S,
    X,
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
    ResposExpunged = 0x40000000,
    ResposDoubleUndo = 0x20000000,
    ResposInvalidate = 0x10000000,
    //
    ResposTransitionC1 = 0x800000,
    ResposTransitionV = 0x400000,
    ResposTransitionW = 0x200000,
    ResposTone = 0x100000,
    ResposAutocorrect = 0x80000,
    //
    ResposMask = 0xff,
    ResposValidMask = 0xff0000,
};

enum class CharTypes : unsigned int {
    Uncategorized = 0,
    Vowel = 1 << 3,
    VowelW = 1 << 4,
    Conso = 1 << 5,
    ConsoC1 = 1 << 5 | 1 << 6,
    ConsoC2 = 1 << 5 | 1 << 7,
    ConsoContinue = 1 << 5 | 1 << 8,
    Tone = 1 << 9,
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
    TelexStates PushChar(_In_ wchar_t c) override;
    TelexStates Backspace() override;
    TelexStates Commit() override;
    TelexStates ForceCommit() override;
    TelexStates Cancel() override;
    TelexStates Backconvert(_In_ const std::wstring& s) override;

    constexpr TelexStates GetState() const override {
        return _state;
    }
    std::wstring Retrieve() const override;
    std::wstring RetrieveRaw() const override;
    std::wstring Peek() const override;
    constexpr std::wstring::size_type Count() const override {
        return _keyBuffer.size();
    }

    constexpr Tones GetTone() const {
        return _t;
    }
    constexpr const std::vector<int>& GetRespos() const {
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

    TelexStates _state = TelexStates::Valid;

    std::wstring _keyBuffer;
    std::wstring _c1;
    std::wstring _v;
    std::wstring _c2;
    Tones _t = Tones::Z;
    int _toneCount = 0;
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
    std::vector<int> _respos;
    int _respos_current = 0;
    bool _backconverted = false;
    bool _autocorrected = false;

private:
    friend struct TelexEngineImpl;
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

    void Invalidate();
    void InvalidateAndPopBack(wchar_t c);
    std::optional<std::pair<std::wstring_view, VInfo>> FindTable() const;
    bool GetTonePos(_In_ bool predict, _Out_ VInfo* vinfo) const;
    void ReapplyTone();
    bool HasValidRespos() const;
};

} // namespace Telex
} // namespace VietType
