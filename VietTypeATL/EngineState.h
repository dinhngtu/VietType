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
#include "Telex.h"

namespace VietType {

class EngineState {
public:
    EngineState(Telex::TelexConfig const& config);

    Telex::TelexEngine& Engine();
    Telex::TelexEngine const& Engine() const;

    void Reset();
    Telex::TELEX_STATES PushChar(_In_ wchar_t c);
    Telex::TELEX_STATES Backspace();
    Telex::TELEX_STATES Commit();
    Telex::TELEX_STATES ForceCommit();
    Telex::TELEX_STATES Cancel();

    std::wstring Retrieve() const;
    std::wstring RetrieveInvalid() const;
    std::wstring Peek() const;
    std::wstring::size_type Count() const;

    int Enabled() const;
    void Enabled(int value);
    int ToggleEnabled();

private:
    int _enabled = 1;
    Telex::TelexEngine _engine;

private:
    DISALLOW_COPY_AND_ASSIGN(EngineState);
};

}
