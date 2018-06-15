#pragma once

#include "stdafx.h"

namespace Telex {
    bool IsEditKey(WPARAM wParam, LPARAM lParam, BYTE const * keyState);
    bool EngineWantsKey(bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState);
    bool EngineWantsKey(bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ SHORT ctrlState, _In_ SHORT menuState);
    Telex::TELEX_STATES PushKey(_In_ Telex::TelexEngine& engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState);
}
