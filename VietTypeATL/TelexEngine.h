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

#include "Common.h"
#include "TelexData.h"
#include "TelexUtil.h"

// A word of warning:
// This class only deals with characters that can be represented by wchar_t.
// For example, German character input (where upper('ß') = 'SS') is not supported,
// nor are CJK languages supported.

namespace VietType {
namespace Telex {
// State transition is as follows:

// VALID (initial) -> VALID|INVALID (by pushing a character)
// INVALID -> INVALID (by pushing a character)

// VALID -> COMMITTED|COMMITTED_INVALID (by calling Commit/ForceCommit)
// INVALID -> COMMITTED_INVALID (by calling Commit/ForceCommit)
// VALID/INVALID -> COMMITTED_INVALID (by calling Cancel)

// COMMITTED: can call Retrieve/RetrieveInvalid, does not change state
// COMMITTED_INVALID: can call RetrieveInvalid, does not change state

// COMMITTED|COMMITTED_INVALID -> VALID (Reset)

enum class TELEX_STATES {
    VALID, // valid word, can accept more chars but cannot get result
    INVALID, // invalid word but can still accept more chars
    COMMITTED, // valid, tones have been applied, cannot accept any more chars (must get result or Reset)
    COMMITTED_INVALID, // invalid, cannot accept any more chars
    TXERROR = -1, // can never be a state, returned when PushChar encounters an error
};

struct TelexConfig {
    bool oa_uy_tone1;
};

class TESTEXPORT TelexEngine {
public:
    TelexEngine(_In_ struct TelexConfig config);
    ~TelexEngine();

    void Reset();
    TELEX_STATES PushChar(_In_ wchar_t c);
    TELEX_STATES Backspace();
    TELEX_STATES Commit();
    TELEX_STATES ForceCommit();
    TELEX_STATES Cancel();

    std::wstring Retrieve() const;
    std::wstring RetrieveInvalid() const;
    std::wstring Peek() const;
    std::wstring::size_type Count() const;

private:
    struct TelexConfig _config;

    TELEX_STATES _state;

    std::wstring _keyBuffer;
    std::wstring _c1;
    std::wstring _v;
    std::wstring _c2;
    TONES _t;
    // don't use vector<bool> since that's special
    /// <summary>
    /// do not use when invalid
    /// 1 = uppercase, 0 = lowercase
    /// </summary>
    std::vector<int> _cases;

private:
    using map_iterator = genmap<std::wstring, VINFO>::const_iterator;
    bool FindTable(_Out_ map_iterator *it) const;

private:
    DISALLOW_COPY_AND_ASSIGN(TelexEngine);
};

}
}
