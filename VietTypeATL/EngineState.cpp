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

#include "EngineState.h"

VietType::EngineState::EngineState(VietType::Telex::TelexConfig const & config) :
    _engine(config) {
}

VietType::Telex::TelexEngine & VietType::EngineState::Engine() {
    return _engine;
}

VietType::Telex::TelexEngine const & VietType::EngineState::Engine() const {
    return _engine;
}

void VietType::EngineState::Reset() {
    _engine.Reset();
}

VietType::Telex::TELEX_STATES VietType::EngineState::PushChar(wchar_t c) {
    return _engine.PushChar(c);
}

VietType::Telex::TELEX_STATES VietType::EngineState::Backspace() {
    return _engine.Backspace();
}

VietType::Telex::TELEX_STATES VietType::EngineState::Commit() {
    return _engine.Commit();
}

VietType::Telex::TELEX_STATES VietType::EngineState::ForceCommit() {
    return _engine.ForceCommit();
}

VietType::Telex::TELEX_STATES VietType::EngineState::Cancel() {
    return _engine.Cancel();
}

std::wstring VietType::EngineState::Retrieve() const {
    return _engine.Retrieve();
}

std::wstring VietType::EngineState::RetrieveInvalid() const {
    return _engine.RetrieveInvalid();
}

std::wstring VietType::EngineState::Peek() const {
    return _engine.Peek();
}

std::wstring::size_type VietType::EngineState::Count() const {
    return _engine.Count();
}

int VietType::EngineState::Enabled() const {
    return _enabled;
}

void VietType::EngineState::Enabled(int value) {
    _enabled = value;
}

int VietType::EngineState::ToggleEnabled() {
    _enabled = !_enabled;
    return _enabled;
}
