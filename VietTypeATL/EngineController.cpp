// SPDX-License-Identifier: GPL-3.0-only

#include "EngineController.h"
#include "LanguageBarHandlers.h"
#include "CompositionManager.h"
#include "EditSessions.h"
#include "Compartment.h"
#include "EngineSettingsController.h"

namespace VietType {

// {B31B741B-63CE-413A-9B5A-D2B69C695A78}
static const GUID GUID_SettingsCompartment_Toggle = {
    0xb31b741b, 0x63ce, 0x413a, {0x9b, 0x5a, 0xd2, 0xb6, 0x9c, 0x69, 0x5a, 0x78}};
// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
static const GUID GUID_LanguageBarButton_Item = {
    0xcca3d390, 0xef1a, 0x4de4, {0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b}};
// {B2FBD2E7-922F-4996-BE77-21085B91A8F0}
static const GUID GUID_SystemNotifyCompartment = {
    0xb2fbd2e7, 0x922f, 0x4996, {0xbe, 0x77, 0x21, 0x8, 0x5b, 0x91, 0xa8, 0xf0}};

STDMETHODIMP EngineController::OnChange(__RPC__in REFGUID rguid) {
    HRESULT hr;

    if (rguid == GUID_SettingsCompartment_Toggle) {
        UpdateStates();
    } else if (rguid == GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) {
        long openclose;
        hr = _openCloseCompartment.GetValue(&openclose);
        assert(hr == S_OK);
        HRESULT_CHECK(hr, L"%s", L"_openCloseCompartment.GetValue failed");
        _blocked = openclose ? _blocked : BlockedKind::Blocked;
        UpdateStates();
    }

    return S_OK;
}

_Check_return_ HRESULT EngineController::Initialize(
    _In_ const std::shared_ptr<Telex::TelexEngine>& engine, _In_ ITfThreadMgr* threadMgr, _In_ TfClientId clientid) {

    HRESULT hr;

    _engine = engine;
    _clientid = clientid;

    hr = threadMgr->QueryInterface(&_langBarItemMgr);
    HRESULT_CHECK_RETURN(hr, L"%s", L"threadMgr->QueryInterface failed");

    // init settings compartment & listener
    hr = CreateInitialize(&_settings, this, threadMgr, clientid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_settings) failed");
    hr = CreateInitialize(
        &_systemNotify, threadMgr, clientid, GUID_SystemNotifyCompartment, true, [this] { return UpdateStates(); });
    DBG_HRESULT_CHECK(hr, L"%s", L"_systemNotify->Initialize failed");

    // GUID_SettingsCompartment_Toggle is global
    hr = CreateInitialize(
        &_enabled, threadMgr, clientid, GUID_SettingsCompartment_Toggle, true, [this] { return UpdateStates(); });
    HRESULT_CHECK_RETURN(hr, L"%s", L"_enabled->Initialize failed");
#ifdef _DEBUG
    long dbgEnabled = 0;
    HRESULT dbgHr = _enabled->GetValueDirect(&dbgEnabled);
    DBG_DPRINT(L"dbgHr %ld dbgEnabled %ld", dbgHr, dbgEnabled);
#endif // _DEBUG

    this->_initialized = true;

    hr = InitLanguageBar();
    HRESULT_CHECK_RETURN(hr, L"%s", L"InitLanguageBar failed");

    // init GUID_COMPARTMENT_KEYBOARD_OPENCLOSE listener

    hr = _openCloseCompartment.Initialize(threadMgr, clientid, GUID_COMPARTMENT_HANDWRITING_OPENCLOSE);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_openCloseCompartment->Initialize failed");

    CComPtr<ITfSource> openCloseSource;
    hr = _openCloseCompartment.GetCompartmentSource(&openCloseSource);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_openCloseCompartment->GetCompartmentSource failed");

    hr = _openCloseCompartmentEventSink.Advise(openCloseSource, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_openCloseCompartmentEventSink.Advise failed");

    return S_OK;
}

HRESULT EngineController::Uninitialize() {
    HRESULT hr;

    hr = _openCloseCompartmentEventSink.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_openCloseCompartmentEventSink.Unadvise failed");

    hr = _openCloseCompartment.Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_openCloseCompartment.Uninitialize failed");

    hr = _enabled->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_enabled->Uninitialize failed");
    _enabled.Release();

    hr = _systemNotify->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_systemNotify->Uninitialize failed");
    _systemNotify.Release();
    hr = _settings->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_settings->Uninitialize failed");
    _settings.Release();

    hr = UninitLanguageBar();
    DBG_HRESULT_CHECK(hr, L"%s", L"UninitLanguageBar failed");

    _langBarItemMgr.Release();
    _engine.reset();

    return S_OK;
}

Telex::TelexEngine& EngineController::GetEngine() {
    return *_engine;
}

const Telex::TelexEngine& EngineController::GetEngine() const {
    return *_engine;
}

DWORD EngineController::IsBackconvertOnBackspace() {
    return _backconvertOnBackspace;
}

_Check_return_ HRESULT EngineController::IsUserEnabled(_Out_ long* penabled) const {
    HRESULT hr;

    hr = _enabled->GetValueOrWriteback(penabled, static_cast<LONG>(_defaultEnabled));
    if (*penabled != 0 && *penabled != -1) {
        DBG_DPRINT(L"resetting enabled from %ld to %ld", *penabled, _defaultEnabled);
        // _enabled may contain garbage value, reset if it's the case
        *penabled = _defaultEnabled == 0 ? 0 : -1;
        DBG_HRESULT_CHECK(_enabled->SetValue(*penabled), L"%s", L"_enabled reset failed");
    }
    return hr;
}

HRESULT EngineController::ToggleUserEnabled() {
    HRESULT hr;

    if (_blocked == BlockedKind::Blocked) {
        DBG_DPRINT("write enabled skipped since engine is blocked");
        return S_OK;
    }

    long enabled;
    hr = this->IsUserEnabled(&enabled);
    HRESULT_CHECK(hr, L"%s", L"this->IsUserEnabled failed");
    if (FAILED(hr)) {
        enabled = false;
    }

    DBG_DPRINT(L"toggling enabled from %ld", enabled);
    hr = _enabled->SetValue(enabled == 0 ? -1 : 0);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_enabled->SetValue failed");
    hr = UpdateStates();
    HRESULT_CHECK_RETURN(hr, L"%s", L"UpdateEnabled failed");

    return S_OK;
}

long EngineController::IsEnabled() const {
    long enabled;
    HRESULT hr = this->IsUserEnabled(&enabled);
    HRESULT_CHECK(hr, L"%s", L"this->IsUserEnabled failed");
    return !!enabled && _blocked == BlockedKind::Free;
}

EngineController::BlockedKind EngineController::GetBlocked() const {
    return _blocked;
}

void EngineController::SetBlocked(_In_ EngineController::BlockedKind blocked) {
    _blocked = blocked;
    UpdateStates();
}

_Check_return_ HRESULT EngineController::GetOpenClose(_Out_ long* openclose) {
    return _openCloseCompartment.GetValue(openclose);
}

EngineSettingsController* EngineController::GetSettings() const {
    return _settings;
}

HRESULT EngineController::UpdateStates() {
    HRESULT hr;

    DBG_DPRINT(L"%s", L"called");

    if (!this->_initialized) {
        return S_FALSE;
    }

    long enabled;
    hr = this->IsUserEnabled(&enabled);
    DBG_HRESULT_CHECK(hr, L"%s", L"this->IsUserEnabled failed");

    DBG_DPRINT(L"enabled = %ld, blocked = %d", enabled, static_cast<int>(_blocked));

    _settings->IsDefaultEnabled(&_defaultEnabled);
    _settings->IsBackconvertOnBackspace(&_backconvertOnBackspace);

    Telex::TelexConfig cfg = GetEngine().GetConfig();
    hr = _settings->LoadTelexSettings(cfg);
    if (SUCCEEDED(hr)) {
        GetEngine().SetConfig(cfg);
    } else {
        DBG_HRESULT_CHECK(hr, L"%s", L"_settings->LoadSettings failed");
    }

    hr = _indicatorButton->Refresh();
    DBG_HRESULT_CHECK(hr, L"%s", L"_indicatorButton->Refresh failed");
    hr = _langBarButton->Refresh();
    DBG_HRESULT_CHECK(hr, L"%s", L"_langBarButton->Refresh failed");

    return S_OK;
}

_Check_return_ HRESULT EngineController::InitLanguageBar() {
    HRESULT hr;

    _indicatorButton = std::make_unique<RefreshableButton>();
    hr = _indicatorButton->Initialize(
        this,
        _langBarItemMgr,
        Globals::GUID_LBI_INPUTMODE,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_indicatorButton->Initialize failed");

    _langBarButton = std::make_unique<RefreshableButton>();
    hr = _langBarButton->Initialize(
        this,
        _langBarItemMgr,
        GUID_LanguageBarButton_Item,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarButton->Initialize failed");

    return S_OK;
}

HRESULT EngineController::UninitLanguageBar() {
    _langBarButton->Uninitialize();
    _langBarButton.reset();

    _indicatorButton->Uninitialize();
    _indicatorButton.reset();

    return S_OK;
}

} // namespace VietType
