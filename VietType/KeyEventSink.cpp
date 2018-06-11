// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Globals.h"
#include "VirtualKeys.h"
#include "IMECore.h"
#include "TestEditSession.h"
#include "stdafx.h"

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
    VK_CATEGORIES keyCat = ClassifyVirtualKey(wParam, lParam);

#ifdef _DEBUG
    if (keyCat == VK_CATEGORIES::DEBUG) {
        *pIsEaten = TRUE;
        DBGPRINT(
            L"debug OnTestKeyDown wParam %zu lParam %zx %s",
            wParam,
            lParam,
            *pIsEaten ? L"eaten" : L"not eaten");
        return S_OK;
    }
#endif

    switch (keyCat) {
    case VK_CATEGORIES::UNCATEGORIZED:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::CHARACTER:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::RELAYOUT:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::MODIFIER:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::EDIT:
        *pIsEaten = FALSE;
        break;
    }

    DBGPRINT(
        L"OnTestKeyDown wParam %zu lParam %zx %s",
        wParam,
        lParam,
        *pIsEaten ? L"eaten" : L"not eaten");

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
    OnTestKeyDown(pContext, wParam, lParam, pIsEaten);
    if (!*pIsEaten) {
        return S_OK;
    }

    VK_CATEGORIES keyCat = ClassifyVirtualKey(wParam, lParam);

#ifdef _DEBUG
    if (keyCat == VK_CATEGORIES::DEBUG) {
        *pIsEaten = FALSE;

        BYTE keyState[256] = { 0 };
        if (!GetKeyboardState(keyState)) {
            return E_FAIL;
        }

        CTestEditSession *session = new (std::nothrow) CTestEditSession(
            this, pContext, 48, MapVirtualKey(48, MAPVK_VK_TO_VSC), keyState, NULL); // "0"
        if (session != nullptr) {
            HRESULT hrSession = S_OK;
            HRESULT hr = pContext->RequestEditSession(
                _tfClientId, session, TF_ES_SYNC | TF_ES_READWRITE, &hrSession);
            session->Release();

            if (FAILED(hr)) {
                DBGPRINT(L"RequestEditSession failed %lx", hr);
                return hr;
            }
        }

        DBGPRINT(
            L"debug OnKeyDown wParam %zu lParam %zx %s",
            wParam,
            lParam,
            *pIsEaten ? L"eaten" : L"not eaten");

        return S_OK;
    }
#endif

    switch (keyCat) {
    case VK_CATEGORIES::UNCATEGORIZED:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::CHARACTER:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::RELAYOUT: {
        // remap not needed any more thanks to English (India) hack
        *pIsEaten = FALSE;
        break;
    }
    case VK_CATEGORIES::MODIFIER:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::EDIT:
        *pIsEaten = FALSE;
        break;
    }

    DBGPRINT(
        L"OnKeyDown wParam %zu lParam %zx %s", wParam, lParam, *pIsEaten ? L"eaten" : L"not eaten");

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI IMECore::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pIsEaten) {
    VK_CATEGORIES keyCat = ClassifyVirtualKey(wParam, lParam);
    switch (keyCat) {
    case VK_CATEGORIES::UNCATEGORIZED:
        *pIsEaten = FALSE;
        break;
    case VK_CATEGORIES::CHARACTER:
    case VK_CATEGORIES::RELAYOUT:
    case VK_CATEGORIES::MODIFIER:
    case VK_CATEGORIES::EDIT:
        //*pIsEaten = TRUE;
        *pIsEaten = TRUE;
        break;
    }

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
    if (wParam == 49) {
        *pIsEaten = TRUE;
    } else {
        *pIsEaten = FALSE;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfKeyEventSink::OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI IMECore::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pIsEaten) {
    pContext;

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

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);

    pKeystrokeMgr->Release();
}
