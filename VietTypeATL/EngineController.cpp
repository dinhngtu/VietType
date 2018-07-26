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
#include "CompositionManager.h"
#include "EditSessions.h"
#include "Compartment.h"

// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
const GUID VietType::GUID_LanguageBarButton_Item = { 0xcca3d390, 0xef1a, 0x4de4, { 0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b } };

VietType::EngineController::EngineController() noexcept {
}

VietType::EngineController::~EngineController() {
}

STDMETHODIMP VietType::EngineController::OnChange(__RPC__in REFGUID rguid) {
    HRESULT hr;

    if (IsEqualGUID(rguid, Globals::GUID_SettingsCompartment_Toggle)) {
        UpdateStates();
    } else if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE)) {
        long openclose;
        hr = _openCloseCompartment->GetValue(&openclose);
        _blocked = openclose ? _blocked : BlockedKind::Blocked;
        UpdateStates();
    }

    return S_OK;
}

_Check_return_ HRESULT VietType::EngineController::Initialize(
    _In_ const std::shared_ptr<Telex::TelexEngine>& engine,
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId clientid) {

    HRESULT hr;

    _engine = engine;
    _clientid = clientid;

    hr = threadMgr->QueryInterface(&_langBarItemMgr);
    HRESULT_CHECK_RETURN(hr, L"%s", L"threadMgr->QueryInterface failed");

    // init settings compartment & listener

    // GUID_SettingsCompartment_Toggle is global
    hr = ConstructInstance(&_settingsCompartment, threadMgr, clientid, Globals::GUID_SettingsCompartment_Toggle, true);
    HRESULT_CHECK_RETURN(hr, L"%s", L"ConstructInstance(&_settingsCompartment) failed");

    // listen to _settingsCompartment
    CComPtr<ITfSource> settingsSource;
    hr = _settingsCompartment->GetCompartmentSource(&settingsSource);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_settingsCompartment->GetCompartmentSource failed");
    auto[settingsSink, hrSettingsSink] = AutoSinkAdvisor<ITfCompartmentEventSink>::AdviseSink(settingsSource.p, this);
    HRESULT_CHECK_RETURN(hrSettingsSink, L"%s", L"AdviseSink(settingsSource) failed");
    _settingsCompartmentEventSink = std::move(settingsSink);

    // init GUID_COMPARTMENT_KEYBOARD_OPENCLOSE listener

    hr = ConstructInstance(&_openCloseCompartment, threadMgr, clientid, GUID_COMPARTMENT_HANDWRITING_OPENCLOSE);
    HRESULT_CHECK_RETURN(hr, L"%s", L"ConstructInstance(&_openCloseCompartment) failed");

    // listen to _openCloseCompartment
    CComPtr<ITfSource> openCloseSource;
    hr = _openCloseCompartment->GetCompartmentSource(&openCloseSource);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_openCloseCompartment->GetCompartmentSource failed");
    auto[openCloseSink, hrOpenCloseSink] = AutoSinkAdvisor<ITfCompartmentEventSink>::AdviseSink(openCloseSource.p, this);
    HRESULT_CHECK_RETURN(hrOpenCloseSink, L"%s", L"AdviseSink(openCloseSource) failed");
    _openCloseCompartmentEventSink = std::move(openCloseSink);

    // langbar

    hr = InitLanguageBar();
    HRESULT_CHECK_RETURN(hr, L"%s", L"InitLanguageBar failed");

    return S_OK;
}

VietType::Telex::TelexEngine& VietType::EngineController::GetEngine() {
    return *_engine;
}

const VietType::Telex::TelexEngine& VietType::EngineController::GetEngine() const {
    return *_engine;
}

_Check_return_ HRESULT VietType::EngineController::IsUserEnabled(_Out_ long* penabled) {
    return CompartmentReadEnabled(penabled);
}

HRESULT VietType::EngineController::WriteUserEnabled(_In_ long enabled) {
    HRESULT hr;

    if (_blocked == BlockedKind::Blocked) {
        DBG_DPRINT("write enabled skipped since engine is blocked");
        return S_OK;
    }

    hr = CompartmentWriteEnabled(enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentWriteEnabled failed");
    hr = UpdateStates();
    HRESULT_CHECK_RETURN(hr, L"%s", L"UpdateEnabled failed");

    return S_OK;
}

HRESULT VietType::EngineController::ToggleUserEnabled() {
    HRESULT hr;
    long enabled;
    hr = CompartmentReadEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentReadEnabled failed");
    return WriteUserEnabled(!enabled);
}

long VietType::EngineController::IsEnabled() const {
    return _enabled;
}

VietType::EngineController::BlockedKind VietType::EngineController::GetBlocked() const {
    return _blocked;
}

void VietType::EngineController::SetBlocked(_In_ VietType::EngineController::BlockedKind blocked) {
    _blocked = blocked;
    UpdateStates();
}

void VietType::EngineController::SetEditBlockedPending() {
    _editBlockedPending = true;
}

bool VietType::EngineController::SetEditBlockedPending(_In_ HRESULT result) {
    _editBlockedPending = _editBlockedPending && FAILED(result);
    return _editBlockedPending;
}

bool VietType::EngineController::IsEditBlockedPending() const {
    return _editBlockedPending;
}

bool VietType::EngineController::ResetEditBlockedPending() {
    bool pending = _editBlockedPending;
    _editBlockedPending = false;
    return pending;
}

_Check_return_ HRESULT VietType::EngineController::GetOpenClose(_Out_ long* openclose) {
    return _openCloseCompartment->GetValue(openclose);
}

HRESULT VietType::EngineController::UpdateStates() {
    HRESULT hr;

    switch (_blocked) {
    case BlockedKind::Free: {
        long enabled;
        hr = CompartmentReadEnabled(&enabled);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentReadEnabled failed");
        DBG_DPRINT(L"block = %d, enabled = %ld", static_cast<int>(_blocked), enabled);
        _enabled = static_cast<bool>(enabled);
        break;
    }
    case BlockedKind::Blocked:
        DBG_DPRINT(L"%s", L"blocked");
        _enabled = false;
        break;
    }

    hr = _indicatorButton->Refresh();
    DBG_HRESULT_CHECK(hr, L"%s", L"_indicatorButton->Refresh failed");
    hr = _langBarButton->Refresh();
    DBG_HRESULT_CHECK(hr, L"%s", L"_langBarButton->Refresh failed");

    return S_OK;
}

_Check_return_ HRESULT VietType::EngineController::CompartmentReadEnabled(_Out_ long* pEnabled) {
    HRESULT hr;

    hr = _settingsCompartment->GetValue(pEnabled);
    if (hr == S_FALSE) {
        hr = CompartmentWriteEnabled(1);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentWriteEnabled failed");
        *pEnabled = 1;
    }

    return hr;
}

HRESULT VietType::EngineController::CompartmentWriteEnabled(_In_ long enabled) {
    return _settingsCompartment->SetValue(enabled);
}

_Check_return_ HRESULT VietType::EngineController::InitLanguageBar() {
    HRESULT hr;

    _indicatorButton = std::make_unique<IndicatorButton>();
    hr = _indicatorButton->Initialize(
        this,
        _langBarItemMgr,
        Globals::GUID_LBI_INPUTMODE,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_indicatorButton->Initialize failed");

    _langBarButton = std::make_unique<LangBarButton>();
    hr = _langBarButton->Initialize(
        this,
        _langBarItemMgr,
        VietType::GUID_LanguageBarButton_Item,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarButton->Initialize failed");

    return S_OK;
}
