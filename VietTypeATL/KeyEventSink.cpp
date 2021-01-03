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
#include "KeyHandler.h"
#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "Utilities.h"

namespace VietType {

// {8CC27CF8-93D2-416C-B1A3-66827F54244A}
static const GUID GUID_KeyEventSink_PreservedKey_Toggle = { 0x8cc27cf8, 0x93d2, 0x416c, { 0xb1, 0xa3, 0x66, 0x82, 0x7f, 0x54, 0x24, 0x4a } };
static const TF_PRESERVEDKEY PK_Toggle = { VK_OEM_3, TF_MOD_ALT }; // Alt-`

// {C5A067B1-D0EF-43C3-80F7-8DB23B56A8FA}
static const GUID GUID_KeyEventSink_PreservedKey_ToggleJA = { 0xc5a067b1, 0xd0ef, 0x43c3, { 0x80, 0xf7, 0x8d, 0xb2, 0x3b, 0x56, 0xa8, 0xfa } };
static const TF_PRESERVEDKEY PK_ToggleJA = { VK_KANJI, TF_MOD_IGNORE_ALL_MODIFIER }; // for Japanese langid

// {FAC88DBE-E799-474B-9A8C-1449CAA38348}
static const GUID GUID_KeyEventSink_PreservedKey_EditBack = { 0xfac88dbe, 0xe799, 0x474b, { 0x9a, 0x8c, 0x14, 0x49, 0xca, 0xa3, 0x83, 0x48 } };
static const TF_PRESERVEDKEY PK_EditBack = { VK_LEFT, TF_MOD_ALT };

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

STDMETHODIMP KeyEventSink::OnTestKeyDown(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }

    *pfEaten = Telex::IsKeyEaten(
        _compositionManager->IsComposing(),
        wParam,
        lParam,
        _keyState);

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && _compositionManager->IsComposing()) {
        hr = CallKeyEdit(pic, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnTestKeyUp(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_compositionManager->IsComposing()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
        }

        *pfEaten = Telex::IsKeyEaten(
            _compositionManager->IsComposing(),
            wParam,
            lParam,
            _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyDown(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }

    *pfEaten = Telex::IsKeyEaten(
        _compositionManager->IsComposing(),
        wParam,
        lParam,
        _keyState);

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    if (*pfEaten || _compositionManager->IsComposing()) {
        hr = CallKeyEdit(pic, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP KeyEventSink::OnKeyUp(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

STDMETHODIMP KeyEventSink::OnPreservedKey(_In_ ITfContext* pic, _In_ REFGUID rguid, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (GUID_KeyEventSink_PreservedKey_Toggle == rguid || GUID_KeyEventSink_PreservedKey_ToggleJA == rguid) {
        *pfEaten = TRUE;
        _controller->GetEngine().Reset();
        hr = _compositionManager->EndComposition();
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EndComposition failed");
        hr = _controller->ToggleUserEnabled();
        DBG_HRESULT_CHECK(hr, L"%s", L"_engine->ToggleEnabled failed");

    } else if (GUID_KeyEventSink_PreservedKey_EditBack == rguid) {
        *pfEaten = TRUE;
        hr = CompositionManager::RequestEditSession(
            EditSessions::EditSurroundingWord,
            _compositionManager,
            pic,
            _controller.p,
            0);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CompositionManager::RequestEditSession failed");
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

    if (Globals::TextServiceLangId == MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN)) {
        hr = _keystrokeMgr->PreserveKey(_clientid, GUID_KeyEventSink_PreservedKey_ToggleJA, &PK_ToggleJA, NULL, 0);
        DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->PreserveKey failed for JA");
    } else {
        hr = _keystrokeMgr->PreserveKey(_clientid, GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle, NULL, 0);
        // probably not fatal
        DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->PreserveKey failed");
    }

    return S_OK;
}

HRESULT KeyEventSink::Uninitialize() {
    HRESULT hr;

    if (Globals::TextServiceLangId == MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN)) {
        hr = _keystrokeMgr->UnpreserveKey(GUID_KeyEventSink_PreservedKey_Toggle, &PK_ToggleJA);
        DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnpreserveKey failed for JA");
    } else {
        hr = _keystrokeMgr->UnpreserveKey(GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle);
        DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnpreserveKey failed");
    }

    hr = _keystrokeMgr->UnadviseKeyEventSink(_clientid);
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnadviseKeyEventSink failed");

    _controller.Release();
    _compositionManager.Release();
    _threadMgr.Release();
    _keystrokeMgr.Release();

    return S_OK;
}

HRESULT KeyEventSink::CallKeyEdit(_In_ ITfContext* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hr;

    CComPtr<KeyHandlerEditSession> keyHandlerEditSession;
    hr = CreateInitialize(&keyHandlerEditSession, _compositionManager, context, wParam, lParam, _keyState, _controller);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&keyHandlerEditSession) failed");
    hr = _compositionManager->RequestEditSession(keyHandlerEditSession, context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->RequestEditSession failed");

    return S_OK;
}

}
