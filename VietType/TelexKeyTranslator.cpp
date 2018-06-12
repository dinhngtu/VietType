#include "TelexKeyTranslator.h"

bool Telex::EngineWantsKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ PBYTE keyState) {
    if ((keyState[VK_CONTROL] & 0x80) || (keyState[VK_MENU] & 0x80)) {
        // engine doesn't want modifiers
        return false;
    }
    if (wParam >= 65 && wParam <= 90) {
        return true;
    }
    if (wParam == 219 || wParam == 221) {
        return true;
    }
    return false;
}

bool Telex::EngineWantsKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ SHORT ctrlState, _In_ SHORT menuState) {
    if ((ctrlState & 0x8000) || (menuState & 0x8000)) {
        // engine doesn't want modifiers
        return false;
    }
    if (wParam >= 65 && wParam <= 90) {
        return true;
    }
    if (wParam == 219 || wParam == 221) {
        return true;
    }
    return false;
}

bool IsTranslatableKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ PBYTE keyState) {
    if (wParam >= 65 && wParam <= 90) {
        return true;
    }
    if (wParam == 219 || wParam == 221) {
        return true;
    }
    return false;
}

bool Telex::TranslateKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ PBYTE keyState, _Out_writes_(1) LPWSTR translated) {
    if (!IsTranslatableKey(wParam, lParam, keyState)) {
        return false;
    }
    return ToUnicode((UINT)wParam, (lParam >> 16) & 0xff, keyState, translated, 1, 0) == 1;
}
