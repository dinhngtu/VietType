// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "Telex.h"

namespace VietType {

bool IsEditKey(_In_ uintptr_t wParam, _In_ intptr_t lParam, _In_reads_(256) const unsigned char* keyState);
bool IsKeyEaten(
    _In_ bool isComposing, _In_ uintptr_t wParam, _In_ intptr_t lParam, _In_reads_(256) const unsigned char* keyState);
Telex::TelexStates PushKey(
    _In_ Telex::TelexEngine& engine,
    _In_ uintptr_t wParam,
    _In_ intptr_t lParam,
    _In_reads_(256) const unsigned char* keyState);

} // namespace VietType
