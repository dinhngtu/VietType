// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Telex.h"
#include "KeyTranslator.h"
#include "ContextManager.h"
#include "Context.h"

namespace VietType {

// {8CC27CF8-93D2-416C-B1A3-66827F54244A}
const GUID GUID_KeyEventSink_PreservedKey_Toggle = {
    0x8cc27cf8, 0x93d2, 0x416c, {0xb1, 0xa3, 0x66, 0x82, 0x7f, 0x54, 0x24, 0x4a}};

STDMETHODIMP ContextManager::OnSetFocus(_In_ BOOL fForeground) {
    HRESULT hr;

    DBG_DPRINT(L"foreground %d", fForeground);

    _focus.Release();

    if (!fForeground) {
        return S_OK;
    }

    CComPtr<ITfDocumentMgr> docMgr;
    hr = _threadMgr->GetFocus(&docMgr);
    if (FAILED(hr)) {
        // we don't care about the error since there might be no focused document manager
        return S_OK;
    }

    return OnSetFocus(docMgr, nullptr);
}

DWORD ContextManager::OnBackconvertBackspace(
    _In_ Context* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten, _In_ DWORD prevBackconvert) {
    bool backconvert = false;

    *pfEaten = FALSE;

    if (wParam == VK_BACK && !context->GetComposition() && !IsModifier(_keyState))
        backconvert = true;

    if (backconvert) {
        *pfEaten = TRUE;
        return prevBackconvert;
    } else {
        *pfEaten = IsKeyEaten(context->GetEngine(), context->GetComposition(), wParam, lParam, _keyState);
        return BackconvertDisabled;
    }
}

DWORD ContextManager::OnBackconvertRetype(
    _In_ Context* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _In_ DWORD prevBackconvert,
    _Out_ wchar_t* acceptedChar) {
    if (!context->GetComposition() && !IsModifier(_keyState) &&
        IsKeyAccepted(context->GetEngine(), wParam, lParam, _keyState, acceptedChar)) {
        *pfEaten = TRUE;
        return prevBackconvert;
    } else {
        *pfEaten = IsKeyEaten(context->GetEngine(), context->GetComposition(), wParam, lParam, _keyState);
        *acceptedChar = 0;
        return 0;
    }
}

HRESULT ContextManager::OnKeyDownCommon(
    _In_ Context* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _Out_ DWORD* isBackconvert,
    _Out_ wchar_t* acceptedChar) {
    *pfEaten = FALSE;
    *isBackconvert = BackconvertDisabled;
    *acceptedChar = 0;

    if (!IsEnabled(context)) {
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
    }

    *isBackconvert = _backconvert;
    switch (*isBackconvert) {
    case BackconvertOnBackspace:
        *isBackconvert = OnBackconvertBackspace(context, wParam, lParam, pfEaten, *isBackconvert);
        break;
    case BackconvertOnType:
        *isBackconvert = OnBackconvertRetype(context, wParam, lParam, pfEaten, *isBackconvert, acceptedChar);
        break;
    default:
        *pfEaten = IsKeyEaten(context->GetEngine(), context->GetComposition(), wParam, lParam, _keyState);
        *isBackconvert = BackconvertDisabled;
        break;
    }

    return S_OK;
}

HRESULT ContextManager::CallKeyEditBackspace(
    _In_ Context* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hr;

    hr = context->RequestEditLastWord(1, L'\0');
    if (FAILED(hr)) {
        DBG_DPRINT(L"backspace SendInput fallback");

        if (context != _focus)
            return E_FAIL;

        INPUT inp[2];
        ZeroMemory(inp, sizeof(inp));
        inp[0].type = inp[1].type = INPUT_KEYBOARD;
        inp[0].ki.wVk = inp[1].ki.wVk = VK_BACK;
        inp[1].ki.dwFlags = KEYEVENTF_KEYUP;
        if (!SendInput(ARRAYSIZE(inp), inp, sizeof(INPUT))) {
            WINERROR_GLE_RETURN_HRESULT(L"SendInput failed");
        }
    }

    return S_OK;
}

HRESULT ContextManager::CallKeyEditRetype(
    _In_ Context* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ wchar_t push) {
    HRESULT hr;

    hr = context->RequestEditLastWord(0, push);
    HRESULT_CHECK_RETURN(hr, L"RequestEditSession EditLastWord failed");

    return S_OK;
}

STDMETHODIMP ContextManager::OnTestKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    DWORD isBackconvert;
    wchar_t c;
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    hr = OnKeyDownCommon(context, wParam, lParam, pfEaten, &isBackconvert, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyDownCommon failed");

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && context->GetComposition()) {
        hr = CallKeyEdit(context, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnTestKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    if (!IsEnabled(context)) {
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (context->GetComposition()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
        }

        *pfEaten = IsKeyEaten(context->GetEngine(), it->second->GetComposition(), wParam, lParam, _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    DWORD isBackconvert;
    wchar_t c;
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    hr = OnKeyDownCommon(context, wParam, lParam, pfEaten, &isBackconvert, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyDownCommon failed");

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    if (*pfEaten || it->second->GetComposition()) {
        switch (isBackconvert) {
        case BackconvertOnBackspace:
            hr = CallKeyEditBackspace(context, wParam, lParam, _keyState);
            break;
        case BackconvertOnType:
            hr = CallKeyEditRetype(context, wParam, lParam, _keyState, c);
            break;
        default:
            hr = CallKeyEdit(context, wParam, lParam, _keyState);
            break;
        }
        HRESULT_CHECK_RETURN(hr, L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

STDMETHODIMP ContextManager::OnPreservedKey(_In_ ITfContext* pic, _In_ REFGUID rguid, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    if (GUID_KeyEventSink_PreservedKey_Toggle == rguid) {
        *pfEaten = TRUE;
        context->GetEngine()->Reset();
        hr = context->EndComposition();
        DBG_HRESULT_CHECK(hr, L"_contextManager->EndComposition failed");
        hr = ToggleUserEnabled();
        DBG_HRESULT_CHECK(hr, L"_engine->ToggleEnabled failed");
    }

    return S_OK;
}

HRESULT ContextManager::CallKeyEdit(
    _In_ Context* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hr, hrSession;
    hr = context->RequestEditKey(&hrSession, wParam, lParam, keyState);
    HRESULT_CHECK_RETURN(hr, L"_contextManager->RequestEditSession failed");
    HRESULT_CHECK_RETURN(hrSession, L"KeyHandlerEditSession failed");

    return S_OK;
}

} // namespace VietType
