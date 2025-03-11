// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "Telex.h"

namespace VietType {

bool IsEditKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);

Telex::TelexStates PushKey(
    _In_ Telex::ITelexEngine* engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);

bool IsKeyEaten(
    _In_ Telex::ITelexEngine* engine,
    _In_ bool isComposing,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState);

} // namespace VietType
