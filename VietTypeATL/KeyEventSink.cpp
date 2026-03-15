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

static inline BOOL IsKeyEaten(KeyResult keyResult) {
    switch (keyResult) {
    case KeyResult::NotEaten:
    case KeyResult::NotEatenEndComposition:
    case KeyResult::BreakingCharacter:
        return FALSE;
    default:
        return TRUE;
    }
}

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

HRESULT ContextManager::OnKeyCommon(
    _In_ Context* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ bool update,
    _Out_ KeyResult* keyResult,
    _Out_ wchar_t* acceptedChar) {
    *keyResult = KeyResult::NotEaten;
    *acceptedChar = 0;

    if (!IsEnabled(context)) {
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
    }

    *keyResult = ClassifyKey(
        context->GetEngine(), context->GetComposition(), _backconvert, wParam, lParam, _keyState, update, acceptedChar);

    return S_OK;
}

HRESULT ContextManager::CallKeyEditBackspace(_In_ Context* context) {
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

HRESULT ContextManager::CallKeyEditRetype(_In_ Context* context, _In_ wchar_t push) {
    HRESULT hr;

    hr = context->RequestEditLastWord(0, push);
    HRESULT_CHECK_RETURN(hr, L"RequestEditSession EditLastWord failed");

    return S_OK;
}

STDMETHODIMP ContextManager::OnTestKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    KeyResult keyResult;
    wchar_t c;
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    hr = OnKeyCommon(context, wParam, lParam, false, &keyResult, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyCommon failed");

    *pfEaten = IsKeyEaten(keyResult);
    return S_OK;
}

STDMETHODIMP ContextManager::OnTestKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    KeyResult keyResult;
    wchar_t c;
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (!context->GetComposition()) {
        return S_OK;
    }

    hr = OnKeyCommon(context, wParam, lParam, false, &keyResult, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyCommon failed");

    *pfEaten = IsKeyEaten(keyResult);
    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    KeyResult keyResult;
    wchar_t c;
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    hr = OnKeyCommon(context, wParam, lParam, true, &keyResult, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyCommon failed");

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, GetKeyResult(keyResult));

    *pfEaten = IsKeyEaten(keyResult);
    hr = S_OK;
    switch (keyResult) {
    case KeyResult::NotEaten:
    case KeyResult::Dropped:
        break;
    case KeyResult::BackconvertingBackspace:
        hr = CallKeyEditBackspace(context);
        break;
    case KeyResult::BackconvertingCharacter:
        hr = CallKeyEditRetype(context, c);
        break;
    case KeyResult::NotEatenEndComposition:
    case KeyResult::BreakingCharacter:
    case KeyResult::ComposingCharacter:
    case KeyResult::ComposingBackspace:
    case KeyResult::ComposingEscape:
        hr = CallKeyEdit(context, keyResult, c);
        break;
    default:
        hr = E_NOTIMPL;
        break;
    }
    HRESULT_CHECK_RETURN(hr, L"edit session failed");

    // swallow the error from the edit session
    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    KeyResult keyResult;
    wchar_t c;
    HRESULT hr;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (!context->GetComposition()) {
        return S_OK;
    }

    hr = OnKeyCommon(context, wParam, lParam, true, &keyResult, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyCommon failed");

    *pfEaten = IsKeyEaten(keyResult);
    return S_OK;
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

HRESULT ContextManager::CallKeyEdit(_In_ Context* context, _In_ KeyResult keyResult, _In_ wchar_t push) {
    HRESULT hr, hrSession;
    auto eaten = IsKeyEaten(keyResult);
    hr = context->RequestEditKey(&hrSession, !eaten, keyResult, push);
    if (!eaten && hr == TF_E_SYNCHRONOUS) {
        DBG_DPRINT(L"fallback to asynchronous composition breaking");
        hr = context->RequestEditKey(&hrSession, false, keyResult, push);
    }
    HRESULT_CHECK_RETURN(hr, L"_contextManager->RequestEditSession failed");
    HRESULT_CHECK_RETURN(hrSession, L"KeyHandlerEditSession failed");

    return S_OK;
}

} // namespace VietType
