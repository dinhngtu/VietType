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

#include "EngineController.h"
#include "LanguageBarHandlers.h"

// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
const GUID VietType::GUID_LanguageBarButton_Item = { 0xcca3d390, 0xef1a, 0x4de4, { 0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b } };

VietType::EngineController::EngineController() {
    _indicatorButton = std::make_unique<IndicatorButton>(this);
    _langBarButton = std::make_unique<LangBarButton>(this);
}

VietType::EngineController::~EngineController() {
}

HRESULT VietType::EngineController::OnChange(REFGUID rguid) {
    if (IsEqualGUID(rguid, Globals::GUID_KeyEventSink_Compartment_Toggle)) {
        UpdateEnabled();
    }

    return S_OK;
}

HRESULT VietType::EngineController::Initialize(
    std::shared_ptr<EngineState> const & engine,
    ITfThreadMgr * threadMgr,
    TfClientId clientid) {

    HRESULT hr;

    _engine = engine;
    _langBarItemMgr = threadMgr;
    _clientid = clientid;

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

    hr = InitLanguageBar();
    HRESULT_CHECK_RETURN(hr, L"%s", L"InitLanguageBar failed");

    return S_OK;
}

HRESULT VietType::EngineController::Uninitialize() {
    HRESULT hr;

    hr = UninitLanguageBar();
    DBG_HRESULT_CHECK(hr, L"%s", L"UninitLanguageBar failed");

    hr = _compartmentEventSink.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_compartmentEventSink.Unadvise failed");

    _compartment.Release();

    _langBarItemMgr.Release();
    _engine.reset();

    return S_OK;
}

VietType::EngineState & VietType::EngineController::GetEngine() {
    return *_engine;
}

VietType::EngineState const & VietType::EngineController::GetEngine() const {
    return *_engine;
}

std::shared_ptr<VietType::EngineState> const & VietType::EngineController::GetEngineShared() {
    return _engine;
}

int VietType::EngineController::IsEnabled() const {
    return _engine->Enabled();
}

HRESULT VietType::EngineController::WriteEnabled(int enabled) {
    HRESULT hr;

    hr = CompartmentWriteEnabled(enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentWriteEnabled failed");
    hr = UpdateEnabled();
    HRESULT_CHECK_RETURN(hr, L"%s", L"UpdateEnabled failed");

    return S_OK;
}

HRESULT VietType::EngineController::ToggleEnabled() {
    return WriteEnabled(!_engine->Enabled());
}

HRESULT VietType::EngineController::UpdateEnabled() {
    HRESULT hr;

    int enabled;
    hr = CompartmentReadEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentReadEnabled failed");
    _engine->Enabled(enabled);
    hr = _indicatorButton->Refresh();
    DBG_HRESULT_CHECK(hr, L"%s", L"_indicatorButton->Refresh failed");
    hr = _langBarButton->Refresh();
    DBG_HRESULT_CHECK(hr, L"%s", L"_langBarButton->Refresh failed");

    return S_OK;
}

HRESULT VietType::EngineController::CompartmentReadEnabled(int * pEnabled) {
    HRESULT hr;

    VARIANT v;
    hr = _compartment->GetValue(&v);
    if (hr == S_FALSE) {
        v.vt = VT_I4;
        v.lVal = 1;
        hr = _compartment->SetValue(_clientid, &v);
        *pEnabled = v.lVal;
        DBG_HRESULT_CHECK(hr, L"%s", L"_compartment->SetValue failed");
    } else if (hr == S_OK) {
        if (v.vt != VT_I4) {
            return E_FAIL;
        }
        *pEnabled = v.lVal;
    }

    return hr;
}

HRESULT VietType::EngineController::CompartmentWriteEnabled(int enabled) {
    HRESULT hr;

    VARIANT v;
    v.vt = VT_I4;
    v.lVal = enabled;
    hr = _compartment->SetValue(_clientid, &v);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compartment->SetValue failed");

    return S_OK;
}

HRESULT VietType::EngineController::InitLanguageBar() {
    HRESULT hr;

    hr = _indicatorButton->Initialize(
        _langBarItemMgr,
        Globals::GUID_LBI_INPUTMODE,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_indicatorButton->Initialize failed");

    hr = _langBarButton->Initialize(
        _langBarItemMgr,
        VietType::GUID_LanguageBarButton_Item,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarButton->Initialize failed");

    return S_OK;
}

HRESULT VietType::EngineController::UninitLanguageBar() {
    _indicatorButton->Uninitialize();
    _langBarButton->Uninitialize();

    return S_OK;
}
