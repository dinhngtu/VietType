// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <string>

namespace VietType {
namespace Telex {

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

enum class TypingStyles : unsigned int {
    Telex,
    Vni,
    TelexComplicated,
    Max,
};

struct TelexConfig {
    TypingStyles typing_style = TypingStyles::Telex;
    // put the tone in "oa"/"uy" in the second character instead of the first
    bool oa_uy_tone1 = true;
    // allow typing the second 'd' of 'dd' anywhere in the word
    bool accept_separate_dd = true;
    // whether backspacing from invalid state preserves that invalid state
    bool backspaced_word_stays_invalid = true;
    // enable certain autocorrect rules
    bool autocorrect = false;
    // optimize key engine for foreign language typing
    unsigned long optimize_multilang = 1;
};

class ITelexEngine {
public:
    virtual ~ITelexEngine() {
    }

    virtual const TelexConfig& GetConfig() const = 0;
    virtual void SetConfig(const TelexConfig& configconfig) = 0;

    virtual void Reset() = 0;
    virtual TelexStates PushChar(wchar_t c) = 0;
    virtual TelexStates Backspace() = 0;
    virtual TelexStates Commit() = 0;
    virtual TelexStates ForceCommit() = 0;
    virtual TelexStates Cancel() = 0;
    virtual TelexStates Backconvert(const std::wstring& s) = 0;

    virtual TelexStates GetState() const = 0;
    virtual std::wstring Retrieve() const = 0;
    virtual std::wstring RetrieveRaw() const = 0;
    virtual std::wstring Peek() const = 0;
    virtual std::wstring::size_type Count() const = 0;

    virtual bool AcceptsChar(wchar_t c) const = 0;
};

ITelexEngine* TelexNew(const TelexConfig&);
void TelexDelete(ITelexEngine*);

} // namespace Telex
} // namespace VietType
