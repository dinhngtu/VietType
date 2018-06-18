#include "Telex.h"
#include "TelexKeyTranslator.h"

bool IsTranslatableKey(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    if (wParam >= 65 && wParam <= 90) {
        return true;
    } else if (wParam == 219 || wParam == 221) {
        return true;
    }
    return false;
}

bool Telex::IsEditKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState) {
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

bool Telex::EngineWantsKey(bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState) {
    if ((keyState[VK_CONTROL] & 0x80) || (keyState[VK_MENU] & 0x80)) {
        // engine doesn't want modifiers
        return false;
    }
    if (IsTranslatableKey(wParam, lParam)) {
        return true;
    }
    if (isComposing && wParam == VK_BACK) {
        return true;
    }
    return false;
}

bool Telex::EngineWantsKey(bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ SHORT ctrlState, _In_ SHORT menuState) {
    if ((ctrlState & 0x8000) || (menuState & 0x8000)) {
        // engine doesn't want modifiers
        return false;
    }
    if (IsTranslatableKey(wParam, lParam)) {
        return true;
    }
    if (isComposing && wParam == VK_BACK) {
        return true;
    }
    return false;
}

Telex::TELEX_STATES Telex::PushKey(_In_ Telex::TelexEngine& engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState) {
    if (IsTranslatableKey(wParam, lParam)) {
        WCHAR c = 0;
        if (ToUnicode((UINT)wParam, (lParam >> 16) & 0xff, keyState, &c, 1, 0) != 1) {
            assert(0);
            return Telex::TELEX_STATES::TXERROR;
        }
        return engine.PushChar(c);
    } else if (wParam == VK_BACK) {
        return engine.Backspace();
    } else {
        return Telex::TELEX_STATES::TXERROR;
    }
}
