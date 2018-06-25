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

enum class MenuItems : UINT {
    About,
};

static std::wstring const MI_About(L"A&bout");

VietType::EngineController::EngineController() {
}

VietType::EngineController::~EngineController() {
}

HRESULT VietType::EngineController::OnClick(TfLBIClick click) {
    if (click == TF_LBI_CLK_LEFT) {
        return ToggleEnabled();
    } else {
        return S_OK;
    }
}

HRESULT VietType::EngineController::InitMenu(ITfMenu * menu) {
    HRESULT hr;

    hr = menu->AddMenuItem(
        static_cast<UINT>(MenuItems::About),
        0,
        NULL,
        NULL,
        MI_About.c_str(),
        static_cast<ULONG>(MI_About.length()),
        NULL);
    HRESULT_CHECK_RETURN(hr, L"%s", L"menu->AddMenuItem MI_About failed");

    return S_OK;
}

HRESULT VietType::EngineController::OnMenuSelect(UINT id) {
    switch (static_cast<MenuItems>(id)) {
    case MenuItems::About:
        MessageBox(NULL, L"VietType", L"VietType", MB_OK | MB_ICONINFORMATION);
        break;
    }
    return S_OK;
}

HRESULT VietType::EngineController::OnChange(REFGUID rguid) {
    if (IsEqualGUID(rguid, Globals::GUID_KeyEventSink_Compartment_Toggle)) {
        UpdateEnabled();
    }

    return S_OK;
}

HRESULT VietType::EngineController::Initialize(
    std::shared_ptr<EngineState> const & engine,
    SmartComObjPtr<LanguageBarButton> const & languageBar,
    ITfThreadMgr * threadMgr,
    TfClientId clientid) {

    HRESULT hr;

    _engine = engine;
    _languageBar = languageBar;

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

    return S_OK;
}

HRESULT VietType::EngineController::Uninitialize() {
    HRESULT hr;

    hr = _compartmentEventSink.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_compartmentEventSink.Unadvise failed");

    _compartment.Release();
    _engine.reset();
    _languageBar.Release();

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
    hr = _languageBar->SetText(enabled ? std::wstring(L"VIE") : std::wstring(L"ENG"));
    HRESULT_CHECK_RETURN(hr, L"%s", L"_languageBar->SetText failed");
    _engine->Enabled(enabled);

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
