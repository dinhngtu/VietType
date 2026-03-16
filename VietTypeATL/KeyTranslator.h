// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "Telex.h"

namespace VietType {

enum BackconvertModes : DWORD {
    BackconvertDisabled = 0,
    BackconvertOnBackspace = 1,
    BackconvertOnType = 2,
};

enum class KeyResult {
    NotEaten,
    NotEatenEndComposition,
    BreakingCharacter,
    Dropped,
    Character,
    Backspace,
    Escape,
};

PCWSTR GetKeyResult(KeyResult keyResult);

KeyResult ClassifyKey(
    _In_ Telex::ITelexEngine* engine,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ bool update,
    _Out_ wchar_t* chr);

} // namespace VietType
