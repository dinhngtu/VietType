// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "Telex.h"

namespace VietType {

class CompositionManager;
class EngineController;

namespace EditSessions {

// EditSessions.cpp

HRESULT EditNextState(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller,
    _In_ Telex::TelexStates state);

HRESULT EditCommit(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller);

// EditBlocked.cpp

HRESULT EditBlocked(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller);

// EditSurroundingWord.cpp

HRESULT EditSurroundingWord(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller,
    _In_ int ignore);

HRESULT EditSurroundingWordAndPush(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller,
    _In_ int ignore,
    _In_ wchar_t push);

} // namespace EditSessions
} // namespace VietType
