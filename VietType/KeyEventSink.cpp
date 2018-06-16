// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Globals.h"
#include "IMECore.h"
#include "TestEditSession.h"
#include "KeyHandlerEditSession.h"
#include "stdafx.h"
#include "TelexKeyTranslator.h"

HRESULT IMECore::_CallKeyEdit(ITfContext *pContext, WPARAM wParam, LPARAM lParam, PBYTE keyState) {
    auto session = new (std::nothrow) KeyHandlerEditSession(this, pContext, wParam, lParam, keyState, _engine);
    if (session != nullptr) {
        HRESULT hrSession = S_OK;
        HRESULT hr = pContext->RequestEditSession(_tfClientId, session, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        session->Release();

        return hr;
    } else {
        return E_FAIL;
    }
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI IMECore::OnSetFocus(BOOL fForeground) {
    fForeground;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyDown
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI
IMECore::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) {
    // potential fix: eat edit keys if composition is under way, then replay them after composition ends (how?)
    DBGPRINT(L"OnTestKeyDown key %x", wParam);

    if (_disabled) {
        *pIsEaten = FALSE;
        return S_OK;
    }

    BYTE keyState[256] = { 0 };
    if (!GetKeyboardState(keyState)) {
        return E_FAIL;
    }
    *pIsEaten = Telex::EngineWantsKey(_IsComposing(), wParam, lParam, keyState);

    // TODO: break off the composition early at OnTestKeyDown so?
    if (!*pIsEaten && _IsComposing()) {
        _CallKeyEdit(pContext, wParam, lParam, keyState);
    }

    DBGPRINT(L"OnTestKeyDown key %x %s", wParam, *pIsEaten ? L"eaten" : L"not eaten");

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI IMECore::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) {
    DBGPRINT(L"OnKeyDown key %x", wParam);

    BYTE keyState[256] = { 0 };
    if (!GetKeyboardState(keyState)) {
        return E_FAIL;
    }
    *pIsEaten = Telex::EngineWantsKey(_IsComposing(), wParam, lParam, keyState);

    DBGPRINT(L"OnKeyDown key %x %s", wParam, *pIsEaten ? L"eaten" : L"not eaten");

    HRESULT hr = _CallKeyEdit(pContext, wParam, lParam, keyState);
    if (FAILED(hr)) {
        return hr;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI IMECore::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) {
    DBGPRINT(L"OnTestKeyUp key %x", wParam);

    if (_disabled) {
        *pIsEaten = FALSE;
        return S_OK;
    }

    // TODO: essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_IsComposing()) {
        BYTE keyState[256] = { 0 };
        if (!GetKeyboardState(keyState)) {
            return E_FAIL;
        }
        *pIsEaten = Telex::EngineWantsKey(_IsComposing(), wParam, lParam, keyState);
    } else {
        *pIsEaten = FALSE;
    }

    DBGPRINT(L"OnTestKeyUp key %x %s", wParam, *pIsEaten ? L"eaten" : L"not eaten");

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pIsEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI IMECore::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) {
    DBGPRINT(L"OnKeyUp key %x", wParam);

    if (_disabled) {
        *pIsEaten = FALSE;
        return S_OK;
    }

    // TODO: essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_IsComposing()) {
        BYTE keyState[256] = { 0 };
        if (!GetKeyboardState(keyState)) {
            return E_FAIL;
        }
        *pIsEaten = Telex::EngineWantsKey(_IsComposing(), wParam, lParam, keyState);
    } else {
        *pIsEaten = FALSE;
    }

    DBGPRINT(L"OnKeyUp key %x %s", wParam, *pIsEaten ? L"eaten" : L"not eaten");

    return S_OK;
}

static TF_PRESERVEDKEY PK_TOGGLEDISABLED = { VK_OEM_3, TF_MOD_ALT }; // Alt-`

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI IMECore::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten) {
    pContext;

    if (IsEqualGUID(Global::IME_PreservedKey_ToggleDisabled, rguid)) {
        DBGPRINT(L"%s", L"eaten toggle key");
        *pIsEaten = TRUE;
        // if still composing, abort it first or we'll have a composition during when no key is eaten
        if (_IsComposing()) {
            _EndComposition(pContext);
        }
        _disabled = !_disabled;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitKeyEventSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL IMECore::_InitKeyEventSink() {
    ITfKeystrokeMgr *pKeystrokeMgr = nullptr;
    HRESULT hr = S_OK;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr))) {
        return FALSE;
    }

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->PreserveKey(_tfClientId, Global::IME_PreservedKey_ToggleDisabled, &PK_TOGGLEDISABLED, NULL, 0);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitKeyEventSink
//
// Unadvise a keystroke sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void IMECore::_UninitKeyEventSink() {
    ITfKeystrokeMgr *pKeystrokeMgr = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr))) {
        return;
    }

    pKeystrokeMgr->UnpreserveKey(Global::IME_PreservedKey_ToggleDisabled, &PK_TOGGLEDISABLED);

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}
