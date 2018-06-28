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

static const TF_PRESERVEDKEY PK_Toggle = { VK_OEM_3, TF_MOD_ALT }; // Alt-`

VietType::KeyEventSink::KeyEventSink() {
}

VietType::KeyEventSink::~KeyEventSink() {
}

STDMETHODIMP VietType::KeyEventSink::OnSetFocus(BOOL fForeground) {
    return S_OK;
}

STDMETHODIMP VietType::KeyEventSink::OnTestKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    HRESULT hr;

    if (!_engine->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }
    *pfEaten = Telex::IsKeyEaten(_compositionManager->IsComposing(), wParam, lParam, _keyState);

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && _compositionManager->IsComposing()) {
        DBG_DPRINT(L"calling key edit wParam = %lx", wParam);
        hr = CallKeyEdit(pic, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP VietType::KeyEventSink::OnTestKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    if (!_engine->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_compositionManager->IsComposing()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
        }
        *pfEaten = Telex::IsKeyEaten(_compositionManager->IsComposing(), wParam, lParam, _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

STDMETHODIMP VietType::KeyEventSink::OnKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    HRESULT hr;

    if (!_engine->IsEnabled()) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }
    *pfEaten = Telex::IsKeyEaten(_compositionManager->IsComposing(), wParam, lParam, _keyState);

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    hr = CallKeyEdit(pic, wParam, lParam, _keyState);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");

    return S_OK;
}

STDMETHODIMP VietType::KeyEventSink::OnKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

STDMETHODIMP VietType::KeyEventSink::OnPreservedKey(ITfContext * pic, REFGUID rguid, BOOL * pfEaten) {
    HRESULT hr;

    if (IsEqualGUID(Globals::GUID_KeyEventSink_PreservedKey_Toggle, rguid)) {
        *pfEaten = TRUE;
        _engine->GetEngine().Reset();
        hr = _compositionManager->EndComposition();
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EndComposition failed");
        hr = _engine->ToggleEnabled();
        DBG_HRESULT_CHECK(hr, L"%s", L"_engine->ToggleEnabled failed");
    }

    return S_OK;
}

HRESULT VietType::KeyEventSink::Initialize(
    ITfThreadMgr * threadMgr,
    TfClientId clientid,
    SmartComObjPtr<CompositionManager> const& compositionManager,
    SmartComObjPtr<EngineController> const& engine) {

    HRESULT hr;

    _clientid = clientid;
    _keystrokeMgr = threadMgr;
    if (!_keystrokeMgr) {
        return E_NOINTERFACE;
    }
    _compositionManager = compositionManager;
    _engine = engine;

    hr = _keystrokeMgr->AdviseKeyEventSink(_clientid, this, TRUE);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keystrokeMgr->AdviseKeyEventSink failed");

    hr = _keystrokeMgr->PreserveKey(_clientid, Globals::GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle, NULL, 0);
    // probably not fatal
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->PreserveKey failed");

    return S_OK;
}

HRESULT VietType::KeyEventSink::Uninitialize() {
    HRESULT hr;

    hr = _keystrokeMgr->UnpreserveKey(Globals::GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle);
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnpreserveKey failed");

    hr = _keystrokeMgr->UnadviseKeyEventSink(_clientid);
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnadviseKeyEventSink failed");

    _engine.Release();
    _compositionManager.Release();
    _keystrokeMgr.Release();

    return S_OK;
}

HRESULT VietType::KeyEventSink::CallKeyEdit(ITfContext *context, WPARAM wParam, LPARAM lParam, BYTE const * keyState) {
    HRESULT hr;

    SmartComObjPtr<KeyHandlerEditSession> keyHandlerEditSession;
    hr = keyHandlerEditSession.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyHandlerEditSession.CreateInstance failed");
    keyHandlerEditSession->Initialize(_compositionManager, context, wParam, lParam, _keyState, _engine->GetEngineShared());
    hr = _compositionManager->RequestEditSession(keyHandlerEditSession, context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->RequestEditSession failed");

    return S_OK;
}
