#pragma once

#include "stdafx.h"

namespace Telex {
    bool EngineWantsKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ PBYTE keyState);
    bool EngineWantsKey(WPARAM wParam, LPARAM lParam, SHORT ctrlState, SHORT menuState);
    bool TranslateKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ PBYTE keyState, _Out_writes_(1) LPWSTR translated);
}
