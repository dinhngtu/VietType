// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "KeyTranslator.h"

namespace VietType {

static _Success_(return) _Check_return_ bool IsTranslatableKey(
    _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState, _Out_opt_ wchar_t* chr) {
    wchar_t c;
    if (ToUnicode((UINT)wParam, (lParam >> 16) & 0xff, keyState, &c, 1, 0) != 1) {
        return false;
    }
    if (chr) {
        *chr = c;
    }
    return true;
}

bool IsEditKey(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
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

bool IsModifier(_In_reads_(256) const BYTE* keyState) {
    return (keyState[VK_CONTROL] & 0x80) || (keyState[VK_MENU] & 0x80) || (keyState[VK_LWIN] & 0x80) ||
           (keyState[VK_RWIN] & 0x80);
}

_Success_(return) bool IsKeyAccepted(
    _In_ Telex::ITelexEngine* engine,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _Out_ wchar_t* c) {
    return IsTranslatableKey(wParam, lParam, keyState, c) && engine->AcceptsChar(*c);
}

bool IsKeyEaten(
    _In_ Telex::ITelexEngine* engine,
    _In_ bool isComposing,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState) {
    if (IsModifier(keyState)) {
        // engine doesn't want modifiers
        return false;
    }
    wchar_t c;
    if (IsKeyAccepted(engine, wParam, lParam, keyState, &c)) {
        return true;
    }
    if (isComposing && (wParam == VK_BACK || wParam == VK_ESCAPE)) {
        return true;
    }
    return false;
}

Telex::TelexStates PushKey(
    _In_ Telex::ITelexEngine* engine, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    wchar_t c;
    if (IsKeyAccepted(engine, wParam, lParam, keyState, &c)) {
        return engine->PushChar(c);
    } else if (wParam == VK_BACK) {
        return engine->Backspace();
    } else {
        return Telex::TelexStates::TxError;
    }
}

} // namespace VietType
