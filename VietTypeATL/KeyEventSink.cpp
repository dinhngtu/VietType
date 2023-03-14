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
static const TF_PRESERVEDKEY PK_Toggle = {VK_OEM_3, TF_MOD_ALT}; // Alt-`

static HRESULT InjectBackspace() {
    /*
    CComPtr<ITfCompartmentMgr> tcMgr;
    hr = pic->QueryInterface(&tcMgr);
    HRESULT_CHECK_RETURN(hr, L"%s", L"pic->QueryInterface failed");
    */
    INPUT inp[2];
    ZeroMemory(inp, sizeof(inp));
    inp[0].type = inp[1].type = INPUT_KEYBOARD;
    inp[0].ki.wVk = inp[1].ki.wVk = VK_BACK;
    inp[1].ki.dwFlags = KEYEVENTF_KEYUP;
    if (!SendInput(ARRAYSIZE(inp), inp, sizeof(INPUT))) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"SendInput failed");
    }
    return S_OK;
}

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
        return S_OK;
    }

    if (!docMgr) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    CComPtr<ITfContext> context;
    hr = docMgr->GetTop(&context);
    if (FAILED(hr)) {
        return S_OK;
    }

    hr = OnNewContext(context, _compositionManager, _controller);
    HRESULT_CHECK_RETURN(hr, L"%s", L"OnNewContext failed");

    return S_OK;
}

/*
 * onkey BS: "nuoc nha " B=-1, KES eats, calls ESW
 * ESW saves word="nha", sets B=4, returns S_FALSE
 * ESW result: KES sees ESW S_FALSE, sends BS
 * onkey BS: "nuoc nha" B=3, KES sends BS
 * onkey BS: "nuoc nh" B=2, KES sends BS
 * onkey BS: "nuoc n" B=1, KES sends BS
 * onkey BS: "nuoc " B=0, KES calls ESW
 * ESW backconverts, creates composition
 * "nuoc (nha)|"
 * ESW successes, sets B=-1, KES completes
 * ================================
 * When to eat backspace? when B=-1 or B=0
 * When to call ESW? when B=-1 or B=0
 * When to inject BS?
 *   KES injects BS when ESW S_FALSE or B>0
 * When to reset compartment?
 */
HRESULT KeyEventSink::OnKeyDownCommon(
    _In_ ITfContext* pic,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _Out_ BOOL* needsESW,
    _Out_ BOOL* needsBS) {
    HRESULT hr;
    long valCompBS = -1;
    *needsESW = FALSE;
    *needsBS = FALSE;

    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }

    Compartment<long> compBackconvert;
    hr = compBackconvert.Initialize(pic, _compositionManager->GetClientId(), Globals::GUID_Compartment_Backconvert);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"compBackconvert.Initialize failed");
        goto finish;
    }

    if (wParam == VK_BACK && !_compositionManager->IsComposing() &&
        !((_keyState[VK_CONTROL] & 0x80) || (_keyState[VK_MENU] & 0x80) || (_keyState[VK_LWIN] & 0x80) ||
          (_keyState[VK_RWIN] & 0x80))) {
        hr = compBackconvert.GetValue(&valCompBS);
        if (FAILED(hr)) {
            DBG_HRESULT_CHECK(hr, L"%s", L"compBackconvert.GetValue failed");
            goto finish;
        }

        *needsESW = _controller->IsBackconvertOnBackspace() && (valCompBS == -1 || valCompBS == 0);
        *needsBS = _controller->IsBackconvertOnBackspace() && (valCompBS > 0);
    }

finish:
    DPRINT(L"backspacing=%ld, needsESW=%d, needsBS=%d", valCompBS, *needsESW, *needsBS);
    if (*needsESW) {
        *pfEaten = TRUE;
    } else {
        if (!needsBS) {
            hr = compBackconvert.SetValue(-1);
            DBG_HRESULT_CHECK(hr, L"%s", L"compBackconvert reset failed");
        }
        *pfEaten = IsKeyEaten(_compositionManager->IsComposing(), wParam, lParam, _keyState);
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    BOOL needsESW, needsBS;
    HRESULT hr = OnKeyDownCommon(pic, wParam, lParam, pfEaten, &needsESW, &needsBS);
    HRESULT_CHECK_RETURN(hr, L"%s", L"OnKeyDownCommon failed");

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && _compositionManager->IsComposing()) {
        hr = CallKeyEdit(pic, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");
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
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
        }

        *pfEaten = IsKeyEaten(_compositionManager->IsComposing(), wParam, lParam, _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    BOOL needsESW, needsBS;
    HRESULT hr = OnKeyDownCommon(pic, wParam, lParam, pfEaten, &needsESW, &needsBS);
    HRESULT_CHECK_RETURN(hr, L"%s", L"OnKeyDownCommon failed");

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    if (needsBS) {
        long backspacing;
        Compartment<long> compBackconvert;
        hr = compBackconvert.Initialize(pic, _compositionManager->GetClientId(), Globals::GUID_Compartment_Backconvert);
        HRESULT_CHECK_RETURN(hr, L"%s", L"compBackconvert.Initialize failed");
        hr = compBackconvert.GetValue(&backspacing);
        HRESULT_CHECK_RETURN(hr, L"%s", L"compBackconvert.GetValue failed");
        hr = compBackconvert.SetValue(backspacing - 1);
        HRESULT_CHECK_RETURN(hr, L"%s", L"compBackconvert.SetValue failed");

        DPRINT(L"%s", L"injecting backspace");
        hr = InjectBackspace();
    } else if (*pfEaten || _compositionManager->IsComposing()) {
        if (needsESW) {
            HRESULT hrSession;
            hr = CompositionManager::RequestEditSessionEx(
                EditSessions::EditSurroundingWord,
                _compositionManager,
                pic,
                TF_ES_SYNC | TF_ES_READWRITE,
                &hrSession,
                _controller.p,
                1);
            if (FAILED(hr) || FAILED(hrSession) || hrSession == S_FALSE) {
                HRESULT_CHECK(hrSession, L"%s", L"backspace SendInput fallback");
                hr = InjectBackspace();
            }
        } else {
            hr = CallKeyEdit(pic, wParam, lParam, _keyState);
            HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");
        }
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
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EndComposition failed");
        hr = _controller->ToggleUserEnabled();
        DBG_HRESULT_CHECK(hr, L"%s", L"_engine->ToggleEnabled failed");
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
    HRESULT_CHECK_RETURN(hr, L"%s", L"threadMgr->QueryInterface failed");

    hr = _keystrokeMgr->AdviseKeyEventSink(_clientid, this, TRUE);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keystrokeMgr->AdviseKeyEventSink failed");

    hr = _keystrokeMgr->PreserveKey(_clientid, GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle, NULL, 0);
    // probably not fatal
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->PreserveKey failed");

    return S_OK;
}

HRESULT KeyEventSink::Uninitialize() {
    HRESULT hr;

    hr = _keystrokeMgr->UnpreserveKey(GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle);
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnpreserveKey failed");

    hr = _keystrokeMgr->UnadviseKeyEventSink(_clientid);
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnadviseKeyEventSink failed");

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
    hr = CreateInitialize(&keyHandlerEditSession, _compositionManager, context, wParam, lParam, _keyState, _controller);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&keyHandlerEditSession) failed");
    hr = _compositionManager->RequestEditSession(keyHandlerEditSession, context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->RequestEditSession failed");

    return S_OK;
}

} // namespace VietType
