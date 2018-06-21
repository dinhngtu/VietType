// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#include "Telex.h"

bool IsTranslatableKey(_In_ WPARAM wParam, _In_ LPARAM lParam) {
    if (wParam >= 65 && wParam <= 90) {
        return true;
    } else if (wParam == 219 || wParam == 221) {
        return true;
    }
    return false;
}

bool VietType::Telex::IsEditKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState) {
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

bool VietType::Telex::EngineWantsKey(bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState) {
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

bool VietType::Telex::EngineWantsKey(bool isComposing, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ SHORT ctrlState, _In_ SHORT menuState) {
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

VietType::Telex::TELEX_STATES VietType::Telex::PushKey(_In_ VietType::Telex::TelexEngine& engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ BYTE const *keyState) {
    if (IsTranslatableKey(wParam, lParam)) {
        WCHAR c = 0;
        if (ToUnicode((UINT)wParam, (lParam >> 16) & 0xff, keyState, &c, 1, 0) != 1) {
            assert(0);
            return VietType::Telex::TELEX_STATES::TXERROR;
        }
        return engine.PushChar(c);
    } else if (wParam == VK_BACK) {
        return engine.Backspace();
    } else {
        return VietType::Telex::TELEX_STATES::TXERROR;
    }
}
