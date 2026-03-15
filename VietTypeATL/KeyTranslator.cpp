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
    case KeyResult::BackconvertingCharacter:
        return L"BackconvertingCharacter";
    case KeyResult::BackconvertingBackspace:
        return L"BackconvertingBackspace";
    case KeyResult::ComposingCharacter:
        return L"ComposingCharacter";
    case KeyResult::ComposingBackspace:
        return L"ComposingBackspace";
    case KeyResult::ComposingEscape:
        return L"ComposingEscape";
    default:
        return L"Unknown";
    }
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

static bool IsKeyEndComposition(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    // only for edit keys that are uneaten and cause compositions to end
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

static _Success_(return) bool IsKeyAcceptedChar(
    _In_ Telex::ITelexEngine* engine,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ bool update,
    _Out_ wchar_t* c) {
    return IsKeyChar(wParam, lParam, keyState, update, c) && engine->AcceptsChar(*c);
}

KeyResult ClassifyKey(
    _In_ Telex::ITelexEngine* engine,
    _In_ bool isComposing,
    _In_ BackconvertModes backconvert,
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
    if (isComposing) {
        if (wParam == VK_BACK) {
            return KeyResult::ComposingBackspace;
        } else if (wParam == VK_ESCAPE) {
            return KeyResult::ComposingEscape;
        }
    } else if (backconvert == BackconvertOnBackspace && wParam == VK_BACK) {
        return KeyResult::BackconvertingBackspace;
    }
    if (IsKeyChar(wParam, lParam, keyState, update, chr)) {
        if (engine->AcceptsChar(*chr)) {
            if (!isComposing && backconvert == BackconvertOnType) {
                return KeyResult::BackconvertingCharacter;
            } else {
                return KeyResult::ComposingCharacter;
            }
        } else {
            return KeyResult::BreakingCharacter;
        }
    } else {
        return KeyResult::NotEaten;
    }
}

Telex::TelexStates PushKey(
    _In_ Telex::ITelexEngine* engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    wchar_t c;
    if (IsKeyAcceptedChar(engine, wParam, lParam, keyState, true, &c)) {
        return engine->PushChar(c);
    } else if (wParam == VK_BACK) {
        return engine->Backspace();
    } else {
        return Telex::TelexStates::TxError;
    }
}

} // namespace VietType
