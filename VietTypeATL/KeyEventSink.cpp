// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// Derived from Microsoft's SampleIME source code included in the Windows classic samples,
// whose original copyright and permission notice is included below:
//
//     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//     ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//     THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//     PARTICULAR PURPOSE.
//
//     Copyright (c) Microsoft Corporation. All rights reserved

#include "KeyEventSink.h"
#include "Telex.h"
#include "KeyTranslator.h"
#include "KeyHandler.h"
#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "EngineSettingsController.h"
#include "ContextUtilities.h"

namespace VietType {

// {8CC27CF8-93D2-416C-B1A3-66827F54244A}
static const GUID GUID_KeyEventSink_PreservedKey_Toggle = {
    0x8cc27cf8, 0x93d2, 0x416c, {0xb1, 0xa3, 0x66, 0x82, 0x7f, 0x54, 0x24, 0x4a}};

STDMETHODIMP KeyEventSink::OnSetFocus(_In_ BOOL fForeground) {
    HRESULT hr;

    DBG_DPRINT(L"foreground %d", fForeground);

    if (!fForeground) {
        return S_OK;
    }

    CComPtr<ITfDocumentMgr> docMgr;
    hr = _threadMgr->GetFocus(&docMgr);
    if (FAILED(hr)) {
        // we don't care about the error since there might be no focused document manager
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    if (!docMgr) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    CComPtr<ITfContext> context;
    hr = docMgr->GetTop(&context);
    if (FAILED(hr)) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    hr = OnNewContext(context, _compositionManager, _controller);
    HRESULT_CHECK_RETURN(hr, L"OnNewContext failed");

    return S_OK;
}

DWORD KeyEventSink::OnBackconvertBackspace(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten, _In_ DWORD prevBackconvert) {
    Compartment<long> compBackconvert;
    HRESULT hr;
    bool backconvert = false;

    *pfEaten = FALSE;

    hr = compBackconvert.Initialize(pic, _compositionManager->GetClientId(), Globals::GUID_Compartment_Backconvert);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"compBackconvert.Initialize failed");
        goto finish;
    }

    if (wParam == VK_BACK && !_compositionManager->IsComposing() && !IsModifier(_keyState)) {
        long backspacing;
        hr = compBackconvert.GetValue(&backspacing);
        if (FAILED(hr)) {
            DBG_HRESULT_CHECK(hr, L"compBackconvert.GetValue failed");
            goto finish;
        }

        backconvert = backspacing != -1;
    }

finish:
    if (backconvert) {
        *pfEaten = TRUE;
        return prevBackconvert;
    } else {
        hr = compBackconvert.SetValue(0);
        DBG_HRESULT_CHECK(hr, L"compBackconvert reset failed");
        *pfEaten = IsKeyEaten(&_controller->GetEngine(), _compositionManager->IsComposing(), wParam, lParam, _keyState);
        return BackconvertDisabled;
    }
}

DWORD KeyEventSink::OnBackconvertRetype(
    _In_ ITfContext* pic,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _In_ DWORD prevBackconvert,
    _Out_ wchar_t* acceptedChar) {
    if (!_compositionManager->IsComposing() && !IsModifier(_keyState) &&
        IsKeyAccepted(&_controller->GetEngine(), wParam, lParam, _keyState, acceptedChar)) {
        *pfEaten = TRUE;
        return prevBackconvert;
    } else {
        *pfEaten = IsKeyEaten(&_controller->GetEngine(), _compositionManager->IsComposing(), wParam, lParam, _keyState);
        *acceptedChar = 0;
        return 0;
    }
}

HRESULT KeyEventSink::OnKeyDownCommon(
    _In_ ITfContext* pic,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _Out_ DWORD* isBackconvert,
    _Out_ wchar_t* acceptedChar) {
    *pfEaten = FALSE;
    *isBackconvert = BackconvertDisabled;
    *acceptedChar = 0;

    if (!_controller->IsEnabled()) {
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
    }

    *isBackconvert = _controller->IsBackconvert();
    switch (*isBackconvert) {
    case BackconvertOnBackspace:
        *isBackconvert = OnBackconvertBackspace(pic, wParam, lParam, pfEaten, *isBackconvert);
        break;
    case BackconvertOnType:
        *isBackconvert = OnBackconvertRetype(pic, wParam, lParam, pfEaten, *isBackconvert, acceptedChar);
        break;
    default:
        *isBackconvert = BackconvertDisabled;
        break;
    }

    return S_OK;
}

HRESULT KeyEventSink::CallKeyEditBackspace(
    _In_ ITfContext* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hrSession;
    HRESULT hr;

    hr = CompositionManager::RequestEditSessionEx(
        EditSessions::EditSurroundingWord,
        _compositionManager,
        context,
        TF_ES_SYNC | TF_ES_READWRITE,
        &hrSession,
        _controller.p,
        1);

    if (FAILED(hr) || FAILED(hrSession)) {
        DBG_DPRINT(L"backspace SendInput fallback");

        CComPtr<ITfCompartmentMgr> tcMgr;
        hr = context->QueryInterface(&tcMgr);
        HRESULT_CHECK_RETURN(hr, L"pic->QueryInterface failed");

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

HRESULT KeyEventSink::CallKeyEditRetype(
    _In_ ITfContext* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ wchar_t push) {
    HRESULT hrSession;
    HRESULT hr;

    hr = CompositionManager::RequestEditSessionEx(
        EditSessions::EditSurroundingWordAndPush,
        _compositionManager,
        context,
        TF_ES_ASYNCDONTCARE | TF_ES_READWRITE,
        &hrSession,
        _controller.p,
        0,
        push);
    HRESULT_CHECK_RETURN(hr, L"RequestEditSession EditSurroundingWord failed");
    HRESULT_CHECK_RETURN(hrSession, L"EditSurroundingWord hrSession failed");

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    DWORD isBackconvert;
    wchar_t c;
    HRESULT hr = OnKeyDownCommon(pic, wParam, lParam, pfEaten, &isBackconvert, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyDownCommon failed");

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && _compositionManager->IsComposing()) {
        hr = CallKeyEdit(pic, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_compositionManager->IsComposing()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
        }

        *pfEaten = IsKeyEaten(&_controller->GetEngine(), _compositionManager->IsComposing(), wParam, lParam, _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    DWORD isBackconvert;
    wchar_t c;
    HRESULT hr = OnKeyDownCommon(pic, wParam, lParam, pfEaten, &isBackconvert, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyDownCommon failed");

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    if (*pfEaten || _compositionManager->IsComposing()) {
        switch (isBackconvert) {
        case BackconvertOnBackspace:
            hr = CallKeyEditBackspace(pic, wParam, lParam, _keyState);
            break;
        case BackconvertOnType:
            hr = CallKeyEditRetype(pic, wParam, lParam, _keyState, c);
            break;
        default:
            hr = CallKeyEdit(pic, wParam, lParam, _keyState);
            break;
        }
        HRESULT_CHECK_RETURN(hr, L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyUp(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

STDMETHODIMP KeyEventSink::OnPreservedKey(_In_ ITfContext* pic, _In_ REFGUID rguid, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (GUID_KeyEventSink_PreservedKey_Toggle == rguid) {
        *pfEaten = TRUE;
        _controller->GetEngine().Reset();
        hr = _compositionManager->EndComposition();
        DBG_HRESULT_CHECK(hr, L"_compositionManager->EndComposition failed");
        hr = _controller->ToggleUserEnabled();
        DBG_HRESULT_CHECK(hr, L"_engine->ToggleEnabled failed");
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

_Check_return_ HRESULT KeyEventSink::Initialize(
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId clientid,
    _In_ CompositionManager* compositionManager,
    _In_ EngineController* engine) {

    HRESULT hr;

    _clientid = clientid;
    _threadMgr = threadMgr;
    _compositionManager = compositionManager;
    _controller = engine;

    hr = threadMgr->QueryInterface(&_keystrokeMgr);
    HRESULT_CHECK_RETURN(hr, L"threadMgr->QueryInterface failed");

    hr = _keystrokeMgr->AdviseKeyEventSink(_clientid, this, TRUE);
    HRESULT_CHECK_RETURN(hr, L"_keystrokeMgr->AdviseKeyEventSink failed");

    _controller->GetSettings()->GetPreservedKeyToggle(&_pk_toggle);
    hr = _keystrokeMgr->PreserveKey(_clientid, GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle, NULL, 0);
    // probably not fatal
    DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->PreserveKey failed");

    return S_OK;
}

HRESULT KeyEventSink::Uninitialize() {
    HRESULT hr;

    hr = _keystrokeMgr->UnpreserveKey(GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle);
    DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->UnpreserveKey failed");

    hr = _keystrokeMgr->UnadviseKeyEventSink(_clientid);
    DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->UnadviseKeyEventSink failed");

    _controller.Release();
    _compositionManager.Release();
    _threadMgr.Release();
    _keystrokeMgr.Release();

    return S_OK;
}

HRESULT KeyEventSink::CallKeyEdit(
    _In_ ITfContext* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hr;

    CComPtr<KeyHandlerEditSession> keyHandlerEditSession;
    hr = CreateInitialize(
        &keyHandlerEditSession, _compositionManager.p, context, _controller.p, wParam, lParam, keyState);
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&keyHandlerEditSession) failed");
    hr = _compositionManager->RequestEditSession(keyHandlerEditSession, context);
    HRESULT_CHECK_RETURN(hr, L"_compositionManager->RequestEditSession failed");

    return S_OK;
}

} // namespace VietType
