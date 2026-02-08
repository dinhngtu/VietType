// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "ContextManager.h"
#include "StatusController.h"
#include "EngineSettingsController.h"
#include "Context.h"
#include "LanguageBarButton.h"

namespace VietType {

// {B31B741B-63CE-413A-9B5A-D2B69C695A78}
static const GUID GUID_Compartment_EnabledToggle = {
    0xb31b741b, 0x63ce, 0x413a, {0x9b, 0x5a, 0xd2, 0xb6, 0x9c, 0x69, 0x5a, 0x78}};

static HRESULT IsContextEmpty(_In_ ITfContext* context, _In_ TfClientId clientid, _Out_ bool* isempty) {
    HRESULT hr;

    Compartment<long> compEmpty;
    hr = compEmpty.Initialize(context, clientid, GUID_COMPARTMENT_EMPTYCONTEXT);
    HRESULT_CHECK_RETURN(hr, L"compEmpty->Initialize failed");

    long contextEmpty;
    hr = compEmpty.GetValue(&contextEmpty);
    HRESULT_CHECK_RETURN(hr, L"compDisabled->GetValue failed");

    *isempty = hr == S_OK && contextEmpty;
    return hr;
}

HRESULT ContextManager::OnFocusContext(_In_opt_ ITfContext* context) {
    HRESULT hr, hrSession;

    if (!context) {
        _focus.Release();
        return S_OK;
    }

    if (_focus && _focus->GetContext() == context) {
        return S_OK;
    }

    hr = OnPushContext(context);
    HRESULT_CHECK_RETURN(hr, L"creating context with OnPushContext failed");
    _focus = _map.at(context);

    bool isempty;
    hr = IsContextEmpty(context, _clientid, &isempty);
    HRESULT_CHECK_RETURN(hr, L"IsContextEmpty failed");
    if (isempty) {
        _focus.Release();
        return S_OK;
    }

#ifdef _DEBUG
    TF_STATUS st;
    hr = context->GetStatus(&st);
    DBG_HRESULT_CHECK(hr, L"context->GetStatus failed") else {
        DBG_DPRINT(
            L"d=%c%c%c%c%c%c%c%c%c s=%c%c%c%c%c%c%c",
            (st.dwDynamicFlags & TS_SD_READONLY) ? L'R' : L'_',
            (st.dwDynamicFlags & TS_SD_LOADING) ? L'L' : L'_',
            (st.dwDynamicFlags & TS_SD_RESERVED) ? L'?' : L'_',
            (st.dwDynamicFlags & TS_SD_TKBAUTOCORRECTENABLE) ? L'A' : L'_',
            (st.dwDynamicFlags & TS_SD_TKBPREDICTIONENABLE) ? L'P' : L'_',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'I' : L'_',
            (st.dwDynamicFlags & TS_SD_INPUTPANEMANUALDISPLAYENABLE) ? L'M' : L'_',
            (st.dwDynamicFlags & TS_SD_EMBEDDEDHANDWRITINGVIEW_ENABLED) ? L'H' : L'_',
            (st.dwDynamicFlags & TS_SD_EMBEDDEDHANDWRITINGVIEW_VISIBLE) ? L'V' : L'_',
            (st.dwStaticFlags & TS_SS_DISJOINTSEL) ? L'D' : L'_',
            (st.dwStaticFlags & TS_SS_REGIONS) ? L'R' : L'_',
            (st.dwStaticFlags & TS_SS_TRANSITORY) ? L'T' : L'_',
            (st.dwStaticFlags & TS_SS_NOHIDDENTEXT) ? L'H' : L'_',
            (st.dwStaticFlags & TS_SS_TKBAUTOCORRECTENABLE) ? L'A' : L'_',
            (st.dwStaticFlags & TS_SS_TKBPREDICTIONENABLE) ? L'P' : L'_',
            (st.dwStaticFlags & TS_SS_UWPCONTROL) ? L'U' : L'_');
    }
#endif

    _focus->GetEngine()->SetConfig(_config);
    hr = _focus->RequestEditBlocked(&hrSession);
    if (FAILED(hr) || FAILED(hrSession)) {
        DBG_HRESULT_CHECK(hr, L"ContextManager::RequestEditSession failed");
    }

    return S_OK;
}

HRESULT ContextManager::OnOpenClose() {
    HRESULT hr;

    if (!this->_initialized) {
        return S_FALSE;
    }

    long enabled;
    hr = this->IsUserEnabled(&enabled);
    DBG_HRESULT_CHECK(hr, L"this->IsUserEnabled failed");

    long openclose;
    hr = _openclose->GetValue(&openclose);
    HRESULT_CHECK_RETURN(hr, L"_openclose->GetValue failed");

    if (!!enabled != !!openclose) {
        hr = _enabled->SetValue(openclose ? 0 : -1);
        HRESULT_CHECK_RETURN(hr, L"_enabled->SetValue failed");

        hr = UpdateStatus(false);
        DBG_HRESULT_CHECK(hr, L"UpdateStatus failed");
    }

    return S_OK;
}

_Check_return_ HRESULT ContextManager::IsUserEnabled(_Out_ long* penabled) const {
    HRESULT hr;

    hr = _enabled->GetValueOrWriteback(penabled, static_cast<LONG>(IsDefaultEnabled()));
    if (*penabled != 0 && *penabled != -1) {
        DBG_DPRINT(L"resetting enabled from %ld to %ld", *penabled, IsDefaultEnabled());
        // _enabled may contain garbage value, reset if it's the case
        *penabled = IsDefaultEnabled() == 0 ? 0 : -1;
        DBG_HRESULT_CHECK(_enabled->SetValue(*penabled), L"_enabled reset failed");
    }
    return hr;
}

HRESULT ContextManager::ToggleUserEnabled() {
    HRESULT hr;

    if (!_focus || _focus->IsBlocked()) {
        DBG_DPRINT("write enabled skipped since engine is blocked");
        return S_OK;
    }

    long enabled;
    hr = IsUserEnabled(&enabled);
    HRESULT_CHECK(hr, L"this->IsUserEnabled failed");
    if (FAILED(hr)) {
        enabled = false;
    }

    DBG_DPRINT(L"toggling enabled from %ld", enabled);
    hr = _enabled->SetValue(enabled == 0 ? -1 : 0);
    HRESULT_CHECK_RETURN(hr, L"_enabled->SetValue failed");
    hr = UpdateStatus(true);
    DBG_HRESULT_CHECK(hr, L"UpdateEnabled failed");

    return S_OK;
}

long ContextManager::IsEnabled(_In_ Context* context) const {
    if (!context)
        return false;
    long enabled;
    HRESULT hr = IsUserEnabled(&enabled);
    HRESULT_CHECK(hr, L"this->IsUserEnabled failed");
    return !!enabled && !context->IsBlocked();
}

HRESULT ContextManager::UpdateStatus(bool foreground) {
    HRESULT hr;

    if (!this->_initialized) {
        return S_FALSE;
    }

    long enabled;
    hr = this->IsUserEnabled(&enabled);
    DBG_HRESULT_CHECK(hr, L"this->IsUserEnabled failed");

    DBG_DPRINT(L"enabled = %ld", enabled);

    if (foreground) {
        hr = _openclose->SetValue(!!enabled);
        DBG_HRESULT_CHECK(hr, L"_openclose->SetValue failed");

        _settings->IsDefaultEnabled(&_defaultEnabled);
        _settings->IsBackconvert(&_backconvert);

        hr = _settings->LoadTelexSettings(_config);
        if (_focus) {
            _focus->GetEngine()->SetConfig(_config);
        }
    }

    hr = _status->UpdateStatus(IsEnabled(_focus), _focus ? _focus->IsBlocked() : true);
    DBG_HRESULT_CHECK(hr, L"_langBarButton->Refresh failed");

    return S_OK;
}

HRESULT ContextManager::UpdateStatus(_In_ Context* context) {
    return UpdateStatus(context == _focus);
}

STDMETHODIMP ContextManager::OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    DBG_DPRINT(L"pdim = %p", pdim);

    return S_OK;
}

STDMETHODIMP ContextManager::OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    HRESULT hr;

    DBG_DPRINT(L"pdim = %p", pdim);

    CComPtr<IEnumTfContexts> contexts;
    hr = pdim->EnumContexts(&contexts);
    if (FAILED(hr)) {
        _map.clear();
        return hr;
    }

    CComPtr<ITfContext> context;
    while (1) {
        ULONG fetched;
        hr = contexts->Next(1, &context, &fetched);
        if (SUCCEEDED(hr) && fetched) {
            _map.erase(context.p);
        } else {
            break;
        }
        context.Release();
    }
    return S_OK;
}

STDMETHODIMP ContextManager::OnSetFocus(
    __RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) {
    HRESULT hr;

    DBG_DPRINT(L"pdimFocus = %p", pdimFocus);

    _focus.Release();

    if (!pdimFocus) {
        return S_OK;
    }

    CComPtr<ITfContext> context;
    hr = pdimFocus->GetTop(&context);
    if (FAILED(hr)) {
        return S_OK;
    }

    hr = OnFocusContext(context);
    HRESULT_CHECK_RETURN(hr, L"OnNewContext failed");

    return S_OK;
}

STDMETHODIMP ContextManager::OnPushContext(__RPC__in_opt ITfContext* pic) {
    HRESULT hr;

    DBG_DPRINT(L"context = %p", pic);

    auto it = _map.lower_bound(pic);
    if (it == _map.end() || _map.key_comp()(pic, it->first) != 0) {
        CComPtr<Context> context;
        hr = CreateInitialize(&context, this, pic, static_cast<const Telex::TelexConfig&>(_config), _displayAtom);
        HRESULT_CHECK_RETURN(hr, "Create Context failed");
        _map.insert(it, {pic, std::move(context)});
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnPopContext(__RPC__in_opt ITfContext* pic) {
    DBG_DPRINT(L"context = %p", pic);

    _map.erase(pic);

    return S_OK;
}

_Check_return_ HRESULT ContextManager::Initialize(
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId clientid,
    _In_ EngineSettingsController* settings,
    _In_ TfGuidAtom displayAtom) {
    HRESULT hr;

    _clientid = clientid;
    _threadMgr = threadMgr;
    _settings = settings;
    _displayAtom = displayAtom;

    hr = CreateInitialize(&_status, this, _threadMgr);
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&_status) failed");

    // GUID_Compartment_EnabledToggle is global
    hr = CreateInitialize(&_enabled, threadMgr, clientid, GUID_Compartment_EnabledToggle, true, [this] {
        HRESULT hr = UpdateStatus(false);
        DBG_HRESULT_CHECK(hr, L"UpdateStatus failed");
        return S_OK;
    });
    HRESULT_CHECK_RETURN(hr, L"_enabled->Initialize failed");
#ifdef _DEBUG
    long dbgEnabled = 0;
    [[maybe_unused]] HRESULT dbgHr = _enabled->GetValueDirect(&dbgEnabled);
    DBG_DPRINT(L"dbgHr %ld dbgEnabled %ld", dbgHr, dbgEnabled);
#endif // _DEBUG

    hr = CreateInitialize(
        &_openclose, threadMgr, clientid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, false, [this] { return OnOpenClose(); });
    DBG_HRESULT_CHECK(hr, L"_openclose->Initialize failed");

    // init settings compartment & listener
    hr = CreateInitialize(&_systemNotify, threadMgr, clientid, Globals::GUID_Compartment_SystemNotify, true, [this] {
        HRESULT hr = UpdateStatus(true);
        DBG_HRESULT_CHECK(hr, L"UpdateStatus failed");
        return S_OK;
    });
    DBG_HRESULT_CHECK(hr, L"_systemNotify->Initialize failed");
    // must cache defaultEnabled early since it's used right away
    _settings->IsDefaultEnabled(&_defaultEnabled);

    long enabled;
    // this already sets enabled state if the compartment is empty
    hr = IsUserEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"_status->IsUserEnabled failed");
    DBG_DPRINT(L"init hr = %ld, enabled = %ld", hr, enabled);

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"_threadMgrEventSinkAdvisor.Advise failed");

    CComPtr<ITfKeystrokeMgr> keystrokeMgr;
    hr = _threadMgr->QueryInterface(&keystrokeMgr);
    if (SUCCEEDED(hr)) {
        hr = keystrokeMgr->AdviseKeyEventSink(_clientid, this, TRUE);
        HRESULT_CHECK_RETURN(hr, L"_keystrokeMgr->AdviseKeyEventSink failed");

        _settings->GetPreservedKeyToggle(&_pk_toggle);
        hr = keystrokeMgr->PreserveKey(_clientid, GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle, NULL, 0);
        // probably not fatal
        DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->PreserveKey failed");
    } else {
        DBG_HRESULT_CHECK(hr, L"_threadMgr->QueryInterface failed");
    }

    _initialized = true;

    return S_OK;
}

HRESULT ContextManager::Uninitialize() {
    HRESULT hr;

    _initialized = false;

    _map.clear();

    if (_threadMgr) {
        CComPtr<ITfKeystrokeMgr> keystrokeMgr;
        hr = _threadMgr->QueryInterface(&keystrokeMgr);
        if (SUCCEEDED(hr)) {
            hr = keystrokeMgr->UnpreserveKey(GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle);
            DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->UnpreserveKey failed");

            hr = keystrokeMgr->UnadviseKeyEventSink(_clientid);
            DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->UnadviseKeyEventSink failed");
        } else {
            DBG_HRESULT_CHECK(hr, L"_threadMgr->QueryInterface failed");
        }
    }

    _threadMgrEventSinkAdvisor.Unadvise();

    if (_systemNotify)
        _systemNotify->Uninitialize();
    _systemNotify.Release();

    if (_settings)
        _settings->Uninitialize();
    _settings.Release();
    if (_openclose)
        _openclose->Uninitialize();
    _openclose.Release();
    if (_enabled)
        _enabled->Uninitialize();
    _enabled.Release();

    if (_status)
        _status->Uninitialize();
    _status.Release();

    _displayAtom = TF_INVALID_GUIDATOM;
    _threadMgr.Release();
    _clientid = TF_CLIENTID_NULL;

    return S_OK;
}

} // namespace VietType
