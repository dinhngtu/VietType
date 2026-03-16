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

HRESULT ContextManager::OnKeyCommon(
    _In_ Context* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_ bool update,
    _Out_ KeyResult* keyResult,
    _Out_ wchar_t* acceptedChar) {
    *keyResult = KeyResult::NotEaten;
    *acceptedChar = 0;

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
    }

    *keyResult = ClassifyKey(context->GetEngine(), wParam, lParam, _keyState, update, acceptedChar);

    return S_OK;
}

HRESULT ContextManager::CallKeyEditBackspace(_In_ Context* context) {
    HRESULT hr;

    hr = context->RequestEditLastWord(1, L'\0');
    HRESULT_CHECK(hr, L"context->RequestEditLastWord failed");
    if (FAILED(hr)) {
        DBG_DPRINT(L"backspace SendInput fallback");

        if (context != _focus)
            return E_FAIL;

        hr = context->InjectKey(VK_BACK);
        HRESULT_CHECK_RETURN(hr, L"context->InjectKey failed");
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
    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    if (!IsEnabled(context)) {
        return S_OK;
    }

    *pfEaten = TRUE;
    return S_OK;
}

STDMETHODIMP ContextManager::OnTestKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    *pfEaten = FALSE;
    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    KeyResult keyResult;
    wchar_t c;
    HRESULT hr, hrSession;

    *pfEaten = FALSE;

    auto it = _map.find(pic);
    if (it == _map.end()) {
        return S_OK;
    }
    auto context = it->second.p;

    if (context->IsInjecting(wParam, lParam)) {
        context->ClearInjecting();
        return S_OK;
    }

    if (!IsEnabled(context)) {
        return S_OK;
    }

    hr = OnKeyCommon(context, wParam, lParam, true, &keyResult, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyCommon failed");

    hr = context->RequestQueryCompositionSync(&hrSession);
    bool hasComposition = SUCCEEDED(hr) && hrSession == S_OK;

    DBG_DPRINT(
        L"OnKeyDown wParam %lx %s RQCS %x %x (%s)",
        wParam,
        GetKeyResult(keyResult),
        hr,
        hrSession,
        hasComposition ? L"has comp" : L"no comp");

    hr = S_OK;
    switch (keyResult) {
    case KeyResult::Dropped:
        *pfEaten = TRUE;
        break;
    case KeyResult::BreakingCharacter:
        if (hasComposition) {
            // in this case, we can eat because we know the char will be appended
            *pfEaten = TRUE;
            // if we know there's a composition, then we need to clear it with an ES
            hr = CallKeyEdit(context, false, keyResult, c);
        } else {
            // if we can't know for sure, then it becomes best-effort as anything we do afterwards risks being wrong
            *pfEaten = FALSE;
            hr = CallKeyEdit(context, false, keyResult, L'\0');
        }
        break;
    case KeyResult::Character:
        *pfEaten = TRUE;
        if (_backconvert == BackconvertOnType) {
            hr = CallKeyEditRetype(context, c);
        } else {
            hr = CallKeyEdit(context, false, keyResult, c);
        }
        break;
    case KeyResult::Backspace:
        if (_backconvert == BackconvertOnBackspace && !hasComposition) {
            *pfEaten = TRUE;
            hr = CallKeyEditBackspace(context);
        } else if (hasComposition) {
            *pfEaten = TRUE;
            hr = CallKeyEdit(context, false, keyResult, c);
        } else {
            *pfEaten = FALSE;
        }
        break;
    case KeyResult::NotEaten:
    case KeyResult::NotEatenEndComposition:
    case KeyResult::Escape:
        *pfEaten = FALSE;
        hr = CallKeyEdit(context, false, keyResult, L'\0');
        break;
    default:
        *pfEaten = FALSE;
        break;
    }
    HRESULT_CHECK(hr, L"edit session failed");

    // swallow the error from the edit session
    return S_OK;
}

STDMETHODIMP ContextManager::OnKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    *pfEaten = FALSE;
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

HRESULT ContextManager::CallKeyEdit(
    _In_ Context* context, _In_ bool sync, _In_ KeyResult keyResult, _In_ wchar_t push) {
    HRESULT hr, hrSession;
    hr = context->RequestEditKey(&hrSession, sync, keyResult, push);
    if (sync && hr == TF_E_SYNCHRONOUS) {
        DBG_DPRINT(L"fallback to asynchronous ES");
        hr = context->RequestEditKey(&hrSession, false, keyResult, push);
    }
    HRESULT_CHECK_RETURN(hr, L"_contextManager->RequestEditSession failed");
    HRESULT_CHECK_RETURN(hrSession, L"KeyHandlerEditSession failed");

    return S_OK;
}

} // namespace VietType
