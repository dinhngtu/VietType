// SPDX-License-Identifier: GPL-3.0-only

#include "Telex.h"

static bool IsTranslatableKey(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    if (wParam >= 65 && wParam <= 90) {
        return true;
    } else if (wParam == 219 || wParam == 221) {
        // square brackets
        return false;
    }
    return false;
}

bool VietType::Telex::IsEditKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    // only for edit keys that don't commit
    if ((keyState[VK_CONTROL] & 0x80) || (keyState[VK_MENU] & 0x80)) {
        return true;
    }
    if (wParam >= 33 && wParam <= 40) {
        return true;
    }
    if (wParam == VK_DELETE) {
        return true;
    }
    return false;
}

bool VietType::Telex::IsKeyEaten(
    _In_ bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    if ((keyState[VK_CONTROL] & 0x80) || (keyState[VK_MENU] & 0x80) || (keyState[VK_LWIN] & 0x80) ||
        (keyState[VK_RWIN] & 0x80)) {
        // engine doesn't want modifiers
        return false;
    }
    if (IsTranslatableKey(wParam, lParam)) {
        return true;
    }
    if (isComposing && (wParam == VK_BACK || wParam == VK_ESCAPE)) {
        return true;
    }
    return false;
}

VietType::Telex::TelexStates VietType::Telex::PushKey(
    _In_ VietType::Telex::TelexEngine& engine,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState) {
    if (IsTranslatableKey(wParam, lParam)) {
        WCHAR c = 0;
        if (ToUnicode((UINT)wParam, (lParam >> 16) & 0xff, keyState, &c, 1, 0) != 1) {
            assert(0);
            return VietType::Telex::TelexStates::TxError;
        }
        return engine.PushChar(c);
    } else if (wParam == VK_BACK) {
        return engine.Backspace();
    } else {
        return VietType::Telex::TelexStates::TxError;
    }
}
