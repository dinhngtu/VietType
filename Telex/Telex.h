// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

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

// State transition is as follows:

// Valid (initial) -> Valid|Invalid (by pushing a character)
// Invalid -> Invalid (by pushing a character)

// Valid -> Committed|CommittedInvalid (by calling Commit/ForceCommit)
// Invalid -> CommittedInvalid (by calling Commit/ForceCommit)
// Valid/Invalid -> CommittedInvalid (by calling Cancel)

// Committed: can call Retrieve/RetrieveRaw, does not change state
// CommittedInvalid: can call Retrieve/RetrieveRaw, does not change state

// Committed|CommittedInvalid -> Valid (Reset)

enum class TelexStates {
    Valid,             // valid word, can accept more chars but cannot get result
    Invalid,           // invalid word but can still accept more chars
    Committed,         // valid, tones have been applied, cannot accept any more chars (must get result or Reset)
    CommittedInvalid,  // invalid, cannot accept any more chars
    BackconvertFailed, // invalid, buffers are desynced, but can backspace
    TxError = -1,      // can never be a state, returned when PushChar encounters an error
};

struct TelexConfig {
    // put the tone in "oa"/"uy" in the second character instead of the first
    bool oa_uy_tone1 = true;
    // allow typing the second 'd' of 'dd' anywhere in the word
    bool accept_separate_dd = true;
    // whether backspacing from invalid state preserves that invalid state
    bool backspaced_word_stays_invalid = true;
    // optimize key engine for foreign language typing
    unsigned long optimize_multilang = 1;
    // enable certain autocorrect rules
    bool autocorrect = false;
};

class TelexEngine {
public:
    explicit TelexEngine(_In_ struct TelexConfig config);
    TelexEngine(const TelexEngine&) = delete;
    TelexEngine& operator=(const TelexEngine&) = delete;
    TelexEngine(TelexEngine&&) = default;
    TelexEngine& operator=(TelexEngine&&) = default;

    const TelexConfig& GetConfig() const;
    void SetConfig(const TelexConfig& config);

    void Reset();
    TelexStates PushChar(_In_ wchar_t c);
    TelexStates Backspace();
    TelexStates Commit();
    TelexStates ForceCommit();
    TelexStates Cancel();
    TelexStates Backconvert(_In_ const std::wstring& s);

    constexpr TelexStates GetState() const {
        return _state;
    }
    std::wstring Retrieve() const;
    std::wstring RetrieveRaw() const;
    std::wstring Peek() const;
    constexpr std::wstring::size_type Count() const {
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
};

} // namespace Telex
} // namespace VietType
