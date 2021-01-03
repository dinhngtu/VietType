// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "TelexEngine.h"

namespace VietType {
namespace Telex {

bool IsEditKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);
bool IsKeyEaten(_In_ bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);
TelexStates PushKey(_In_ TelexEngine& engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);

}
}
