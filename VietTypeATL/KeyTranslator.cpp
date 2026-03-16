// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "KeyTranslator.h"

namespace VietType {

PCWSTR GetKeyResult(KeyResult keyResult) {
    switch (keyResult) {
    case KeyResult::NotEaten:
        return L"NotEaten";
    case KeyResult::NotEatenEndComposition:
        return L"NotEatenEndComposition";
    case KeyResult::BreakingCharacter:
        return L"BreakingCharacter";
    case KeyResult::Dropped:
        return L"Dropped";
    case KeyResult::Character:
        return L"Character";
    case KeyResult::Backspace:
        return L"Backspace";
    case KeyResult::Escape:
        return L"Escape";
    default:
        return L"Unknown";
    }
}

static bool IsKeyEndComposition(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    // only for control keys that are uneaten, cause compositions to end, and don't come with a character
    if ((keyState[VK_CONTROL] & 0x80) || (keyState[VK_MENU] & 0x80)) {
        return true;
    }
    if (wParam >= VK_PRIOR && wParam <= VK_DOWN) {
        return true;
    }
    if (wParam == VK_TAB || wParam == VK_DELETE) {
        return true;
    }
    return false;
}

static _Success_(return) _Check_return_ bool IsKeyChar(
    _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState, bool update, _Out_opt_ wchar_t* chr) {
    wchar_t c;
    if (ToUnicode((UINT)wParam, LOBYTE(HIWORD(lParam)), keyState, &c, 1, update ? 0x4 : 0) != 1) {
        return false;
    }
    if (chr) {
        *chr = c;
    }
    return true;
}

KeyResult ClassifyKey(
    _In_ Telex::ITelexEngine* engine,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ bool update,
    _Out_ wchar_t* chr) {
    *chr = 0;
    if (IsKeyEndComposition(wParam, lParam, keyState)) {
        return KeyResult::NotEatenEndComposition;
    }
    if ((keyState[VK_LWIN] & 0x80) || (keyState[VK_RWIN] & 0x80)) {
        // engine doesn't want modifiers, but the other modifiers are already checked in IsKeyEndComposition
        return KeyResult::NotEaten;
    }
    if (wParam == VK_BACK) {
        return KeyResult::Backspace;
    } else if (wParam == VK_ESCAPE) {
        return KeyResult::Escape;
    }
    if (IsKeyChar(wParam, lParam, keyState, update, chr)) {
        if (engine->AcceptsChar(*chr)) {
            return KeyResult::Character;
        } else {
            return KeyResult::BreakingCharacter;
        }
    } else {
        return KeyResult::NotEatenEndComposition;
    }
}

} // namespace VietType
