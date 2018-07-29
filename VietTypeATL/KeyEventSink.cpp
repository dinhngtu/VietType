// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "KeyEventSink.h"
#include "Telex.h"
#include "KeyHandler.h"
#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "Utilities.h"

// {8CC27CF8-93D2-416C-B1A3-66827F54244A}
static const GUID GUID_KeyEventSink_PreservedKey_Toggle = { 0x8cc27cf8, 0x93d2, 0x416c, { 0xb1, 0xa3, 0x66, 0x82, 0x7f, 0x54, 0x24, 0x4a } };
static const TF_PRESERVEDKEY PK_Toggle = { VK_OEM_3, TF_MOD_ALT }; // Alt-`

// {FAC88DBE-E799-474B-9A8C-1449CAA38348}
static const GUID GUID_KeyEventSink_PreservedKey_EditBack = { 0xfac88dbe, 0xe799, 0x474b, { 0x9a, 0x8c, 0x14, 0x49, 0xca, 0xa3, 0x83, 0x48 } };
static const TF_PRESERVEDKEY PK_EditBack = { VK_LEFT, TF_MOD_ALT };

VietType::KeyEventSink::KeyEventSink() noexcept {
}

VietType::KeyEventSink::~KeyEventSink() {
}

STDMETHODIMP VietType::KeyEventSink::OnSetFocus(_In_ BOOL fForeground) {
    HRESULT hr;

    DBG_DPRINT(L"foreground %d, pending %d", fForeground, _controller->IsEditBlockedPending());

    if (!fForeground || _controller->IsEditBlockedPending()) {
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

    bool isempty;
    hr = VietType::IsContextEmpty(context, _compositionManager->GetClientId(), &isempty);
    HRESULT_CHECK_RETURN(hr, L"%s", L"VietType::IsContextEmpty failed");
    if (isempty) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    TF_STATUS st;
    hr = context->GetStatus(&st);
    if (SUCCEEDED(hr)) {
        DBG_DPRINT(
            L"d=%c%c%c s=%c%c%c",
            (st.dwDynamicFlags & TF_SD_LOADING) ? L'L' : L'_',
            (st.dwDynamicFlags & TF_SD_READONLY) ? L'R' : L'_',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'U' : L'_',
            (st.dwStaticFlags & TF_SS_DISJOINTSEL) ? L'D' : L'_',
            (st.dwStaticFlags & TF_SS_REGIONS) ? L'R' : L'_',
            (st.dwStaticFlags & TF_SS_TRANSITORY) ? L'T' : L'_');
    } else DBG_HRESULT_CHECK(hr, L"%s", L"context->GetStatus failed");

    if (!_controller->IsEditBlockedPending()) {
        _controller->SetEditBlockedPending();
        hr = CompositionManager::RequestEditSession(
            VietType::EditSessions::EditBlocked,
            _compositionManager,
            context,
            _controller.p);
        _controller->ResetEditBlockedPending();
        if (FAILED(hr)) {
            DBG_HRESULT_CHECK(hr, L"%s", L"CompositionManager::RequestEditSession failed");
            _controller->SetBlocked(EngineController::BlockedKind::Free);
        }
    }

    return S_OK;
}

STDMETHODIMP VietType::KeyEventSink::OnTestKeyDown(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
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

STDMETHODIMP VietType::KeyEventSink::OnTestKeyUp(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_compositionManager->IsComposing()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
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

STDMETHODIMP VietType::KeyEventSink::OnKeyDown(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (!_controller->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }

    *pfEaten = Telex::IsKeyEaten(
        _compositionManager->IsComposing(),
        wParam,
        lParam,
        _keyState);

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    hr = CallKeyEdit(pic, wParam, lParam, _keyState);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");

    return S_OK;
}

STDMETHODIMP VietType::KeyEventSink::OnKeyUp(_In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

STDMETHODIMP VietType::KeyEventSink::OnPreservedKey(_In_ ITfContext* pic, _In_ REFGUID rguid, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (IsEqualGUID(GUID_KeyEventSink_PreservedKey_Toggle, rguid)) {
        *pfEaten = TRUE;
        _controller->GetEngine().Reset();
        hr = _compositionManager->EndComposition();
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EndComposition failed");
        hr = _controller->ToggleUserEnabled();
        DBG_HRESULT_CHECK(hr, L"%s", L"_engine->ToggleEnabled failed");

    } else if (IsEqualGUID(GUID_KeyEventSink_PreservedKey_EditBack, rguid)) {
        *pfEaten = TRUE;
        hr = CompositionManager::RequestEditSession(
            VietType::EditSessions::EditSurroundingWord,
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

_Check_return_ HRESULT VietType::KeyEventSink::Initialize(
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

HRESULT VietType::KeyEventSink::Uninitialize() {
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

HRESULT VietType::KeyEventSink::CallKeyEdit(_In_ ITfContext* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hr;

    CComPtr<KeyHandlerEditSession> keyHandlerEditSession;
    hr = CreateInstance2(&keyHandlerEditSession);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyHandlerEditSession.CreateInstance failed");
    keyHandlerEditSession->Initialize(_compositionManager, context, wParam, lParam, _keyState, _controller);
    hr = _compositionManager->RequestEditSession(keyHandlerEditSession, context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->RequestEditSession failed");

    return S_OK;
}
