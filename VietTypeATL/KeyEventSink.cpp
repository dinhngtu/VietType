// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

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

#include "KeyEventSink.h"
#include "Telex.h"
#include "KeyHandler.h"

static const TF_PRESERVEDKEY PK_Toggle = { VK_OEM_3, TF_MOD_ALT }; // Alt-`

VietType::KeyEventSink::KeyEventSink() {
}

VietType::KeyEventSink::~KeyEventSink() {
}

HRESULT VietType::KeyEventSink::OnSetFocus(BOOL fForeground) {
    return S_OK;
}

HRESULT VietType::KeyEventSink::OnTestKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    HRESULT hr;

    if (!_enabled) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }
    *pfEaten = Telex::EngineWantsKey(_compositionManager->IsComposing(), wParam, lParam, _keyState);

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && _compositionManager->IsComposing()) {
        hr = CallKeyEdit(pic, wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");
    }

    return S_OK;
}

HRESULT VietType::KeyEventSink::OnTestKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    if (!_enabled) {
        *pfEaten = FALSE;
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (_compositionManager->IsComposing()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
        }
        *pfEaten = Telex::EngineWantsKey(_compositionManager->IsComposing(), wParam, lParam, _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

HRESULT VietType::KeyEventSink::OnKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    HRESULT hr;

    if (!_enabled) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetKeyboardState failed");
    }
    *pfEaten = Telex::EngineWantsKey(_compositionManager->IsComposing(), wParam, lParam, _keyState);

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    hr = CallKeyEdit(pic, wParam, lParam, _keyState);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CallKeyEdit failed");

    return S_OK;
}

HRESULT VietType::KeyEventSink::OnKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

HRESULT VietType::KeyEventSink::OnPreservedKey(ITfContext * pic, REFGUID rguid, BOOL * pfEaten) {
    if (IsEqualGUID(Globals::GUID_KeyEventSink_PreservedKey_Toggle, rguid)) {
        *pfEaten = TRUE;
        _compositionManager->EndComposition();
        _enabled = !_enabled;
        WriteEnabled(_enabled);
    }

    return S_OK;
}

HRESULT VietType::KeyEventSink::OnChange(REFGUID rguid) {
    if (IsEqualGUID(rguid, Globals::GUID_KeyEventSink_Compartment_Toggle)) {
        ReadEnabled(&_enabled);
    }

    return S_OK;
}

HRESULT VietType::KeyEventSink::Initialize(
    ITfThreadMgr * threadMgr,
    TfClientId clientid,
    SmartComObjPtr<CompositionManager> const& compositionManager,
    std::shared_ptr<Telex::TelexEngine> const& engine) {

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

    // deadly but not quite fatal
    SmartComPtr<ITfCompartmentMgr> compartmentMgr;
    hr = threadMgr->GetGlobalCompartment(compartmentMgr.GetAddress());
    if (SUCCEEDED(hr)) {

        hr = compartmentMgr->GetCompartment(Globals::GUID_KeyEventSink_Compartment_Toggle, _compartment.GetAddress());
        if (SUCCEEDED(hr)) {

            SmartComPtr<ITfSource> compartmentSource(_compartment);
            if (!compartmentSource) {
                return E_FAIL;
            }
            hr = _compartmentEventSink.Advise(compartmentSource, this);
            DBG_HRESULT_CHECK(hr, L"%s", L"_compartmentEventSink.Advise failed");

        } else DBG_HRESULT_CHECK(hr, L"%s", L"compartmentMgr->GetCompartment failed");

    } else DBG_HRESULT_CHECK(hr, L"%s", L"threadMgr->GetGlobalCompartment failed");

    return S_OK;
}

HRESULT VietType::KeyEventSink::Uninitialize() {
    HRESULT hr;

    hr = _compartmentEventSink.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_compartmentEventSink.Unadvise failed");

    hr = _keystrokeMgr->UnpreserveKey(Globals::GUID_KeyEventSink_PreservedKey_Toggle, &PK_Toggle);
    DBG_HRESULT_CHECK(hr, L"%s", L"_keystrokeMgr->UnpreserveKey failed");

    hr = _keystrokeMgr->UnadviseKeyEventSink(_clientid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keystrokeMgr->UnadviseKeyEventSink failed");

    _compositionManager.Release();

    return S_OK;
}

HRESULT VietType::KeyEventSink::ReadEnabled(int * pEnabled) {
    HRESULT hr;

    VARIANT v;
    hr = _compartment->GetValue(&v);
    if (hr == S_FALSE) {
        v.vt = VT_I4;
        v.lVal = 1;
        hr = _compartment->SetValue(_clientid, &v);
        DBG_HRESULT_CHECK(hr, L"%s", L"_compartment->SetValue failed");
    } else if (hr == S_OK) {
        if (v.vt != VT_I4) {
            return E_FAIL;
        }
        *pEnabled = v.lVal;
    }

    return hr;
}

HRESULT VietType::KeyEventSink::WriteEnabled(int enabled) {
    HRESULT hr;

    VARIANT v;
    v.vt = VT_I4;
    v.lVal = enabled;
    hr = _compartment->SetValue(_clientid, &v);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compartment->SetValue failed");

    return S_OK;
}

HRESULT VietType::KeyEventSink::CallKeyEdit(ITfContext *context, WPARAM wParam, LPARAM lParam, BYTE const * keyState) {
    HRESULT hr;

    SmartComObjPtr<KeyHandlerEditSession> keyHandlerEditSession;
    hr = keyHandlerEditSession.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyHandlerEditSession.CreateInstance failed");
    keyHandlerEditSession->Initialize(_compositionManager, context, wParam, lParam, _keyState, _engine);
    hr = _compositionManager->RequestEditSession(keyHandlerEditSession);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->RequestEditSession failed");

    return S_OK;
}
