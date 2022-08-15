// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "TelexEngine.h"

namespace VietType {
namespace Telex {

bool IsEditKey(_In_ uintptr_t wParam, _In_ intptr_t lParam, _In_reads_(256) const unsigned char* keyState);
bool IsKeyEaten(
    _In_ bool isComposing, _In_ uintptr_t wParam, _In_ intptr_t lParam, _In_reads_(256) const unsigned char* keyState);
TelexStates PushKey(
    _In_ TelexEngine& engine,
    _In_ uintptr_t wParam,
    _In_ intptr_t lParam,
    _In_reads_(256) const unsigned char* keyState);

} // namespace Telex
} // namespace VietType
