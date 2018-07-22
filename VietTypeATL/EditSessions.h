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

namespace VietType {

class CompositionManager;
class EngineController;

class EditSessions {
public:
    static HRESULT EditBlocked(
        TfEditCookie ec,
        CompositionManager *compositionManager,
        ITfContext *context,
        EngineController *controller);

    static HRESULT EditSurroundingWord(
        _In_ TfEditCookie ec,
        _In_ VietType::CompositionManager *compositionManager,
        _In_ ITfContext *context,
        _In_ VietType::EngineController *controller,
        _In_ int ignore);

private:
    DISALLOW_IMPLICIT_CONSTRUCTORS(EditSessions);
};

}
