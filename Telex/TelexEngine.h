// Copyright (c) 2018 Dinh Ngoc Tu.
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

#include "Common.h"
#include "TelexInternal.h"

namespace VietType {
namespace Telex {
// State transition is as follows:

// Valid (initial) -> Valid|Invalid (by pushing a character)
// Invalid -> Invalid (by pushing a character)

// Valid -> Committed|CommittedInvalid (by calling Commit/ForceCommit)
// Invalid -> CommittedInvalid (by calling Commit/ForceCommit)
// Valid/Invalid -> CommittedInvalid (by calling Cancel)

// Committed: can call Retrieve/RetrieveInvalid, does not change state
// CommittedInvalid: can call RetrieveInvalid, does not change state

// Committed|CommittedInvalid -> Valid (Reset)

enum class TelexStates {
    Valid, // valid word, can accept more chars but cannot get result
    Invalid, // invalid word but can still accept more chars
    Committed, // valid, tones have been applied, cannot accept any more chars (must get result or Reset)
    CommittedInvalid, // invalid, cannot accept any more chars
    TxError = -1, // can never be a state, returned when PushChar encounters an error
};

struct TelexConfig {
    // put the tone in "oa"/"uy" in the second character instead of the first
    bool oa_uy_tone1 = true;
    // allow typing the second 'd' of 'dd' anywhere in the word
    bool accept_separate_dd = true;
};

class TelexEngine {
public:
    explicit TelexEngine(_In_ struct TelexConfig config);
    TelexEngine(const TelexEngine&) = delete;
    TelexEngine& operator=(const TelexEngine&) = delete;

    const TelexConfig& GetConfig() const;
    void SetConfig(const TelexConfig& config);

    void Reset();
    TelexStates PushChar(_In_ wchar_t c);
    TelexStates Backspace();
    TelexStates Commit();
    TelexStates ForceCommit();
    TelexStates Cancel();
    TelexStates Backconvert(_In_ const std::wstring& s);

    TelexStates GetState() const;
    std::wstring Retrieve() const;
    std::wstring RetrieveInvalid() const;
    std::wstring Peek() const;
    std::wstring::size_type Count() const;

private:
    struct TelexConfig _config;

    TelexStates _state = TelexStates::Valid;

    std::wstring _keyBuffer;
    std::wstring _c1;
    std::wstring _v;
    std::wstring _c2;
    Tones _t = Tones::Z;
    // don't use bool vector since that's special cased in the STL
    /// <summary>
    /// only use when valid;
    /// 1 = uppercase, 0 = lowercase
    /// </summary>
    std::vector<int> _cases;
    /// <summary>
    /// only use when valid;
    /// for each character in the _keyBuffer, record which output character it's responsible for,
    /// ex. 'đuống' (dduoongs) _respos = 0C12V34T (T = tone, C = transition _c1, V = transition _v)
    /// </summary>
    std::vector<int> _respos;
    int _respos_current = 0;

private:
    using map_iterator = generic_map_type<std::wstring, VInfo>::const_iterator;
    bool FindTable(_Out_ map_iterator *it) const;
    bool GetTonePos(_In_ bool predict, _Out_ VInfo *vinfo) const;

private:
    bool CheckInvariants() const;
};

}
}
