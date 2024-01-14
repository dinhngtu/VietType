// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class CompositionManager;
class EngineController;

namespace EditSessions {

HRESULT EditBlocked(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller);

HRESULT EditSurroundingWord(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller,
    _In_ int ignore);

} // namespace EditSessions
} // namespace VietType
