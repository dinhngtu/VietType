// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "ContextManager.h"
#include "StatusController.h"
#include "EngineSettingsController.h"
#include "Context.h"
#include "LanguageBarButton.h"

namespace VietType {

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

HRESULT ContextManager::OnToggle(bool fromOpenClose) {
    long enabled;
    HRESULT hr;

    if (!this->_initialized) {
        return S_FALSE;
    }

    if (fromOpenClose) {
        hr = _openclose->GetValue(&enabled);
        HRESULT_CHECK_RETURN(hr, L"_openclose->GetValue failed");
    } else {
        hr = _enabled->GetValue(&enabled, _defaultEnabled);
        HRESULT_CHECK_RETURN(hr, L"_enabled->GetValue failed");
    }

    hr = _openclose->SetValue(enabled);
    HRESULT_CHECK_RETURN(hr, L"_openclose->SetValue failed");
    hr = _enabled->SetValue(enabled);
    HRESULT_CHECK_RETURN(hr, L"_enabled->SetValue failed");

    hr = UpdateStatus(fromOpenClose);
    DBG_HRESULT_CHECK(hr, L"UpdateStatus failed");

    return S_OK;
}

HRESULT ContextManager::OnSettingsChange() {
    Telex::TelexConfig newConfig;
    HRESULT hr;

    _settings->IsDefaultEnabled(&_defaultEnabled);
    _settings->IsBackconvert(reinterpret_cast<DWORD*>(&_backconvert));

    hr = _settings->LoadTelexSettings(newConfig);
    HRESULT_CHECK(hr, L"LoadTelexSettings failed");
    if (SUCCEEDED(hr)) {
        _config = newConfig;
        _configVersion++;
    }

    hr = UpdateStatus(false);
    DBG_HRESULT_CHECK(hr, L"UpdateStatus failed");

    return S_OK;
}

HRESULT ContextManager::ToggleUserEnabled() {
    long enabled;
    HRESULT hr;

    if (!_focus || _focus->IsBlocked()) {
        DBG_DPRINT("write enabled skipped since engine is blocked");
        return S_OK;
    }

    hr = _enabled->GetValue(&enabled, !_defaultEnabled);
    HRESULT_CHECK_RETURN(hr, L"_enabled->GetValue failed");

    enabled = !enabled;
    hr = _openclose->SetValue(enabled);
    HRESULT_CHECK_RETURN(hr, L"_openclose->SetValue failed");
    hr = _enabled->SetValue(enabled);
    HRESULT_CHECK_RETURN(hr, L"_enabled->SetValue failed");

    hr = UpdateStatus(true);
    DBG_HRESULT_CHECK(hr, L"UpdateEnabled failed");

    return S_OK;
}

bool ContextManager::IsEnabled(_In_ Context* context) const {
    HRESULT hr;

    if (!context) {
        return false;
    }

    long openclose;
    hr = _openclose->GetValue(&openclose);
    HRESULT_CHECK(hr, L"_openclose->GetValue failed");
    if (FAILED(hr) || !openclose) {
        return false;
    }

    long disabled;
    Compartment<long> compDisabled;
    hr = compDisabled.Initialize(context->GetContext(), _clientid, GUID_COMPARTMENT_KEYBOARD_DISABLED);
    HRESULT_CHECK(hr, L"compDisabled.Initialize failed");
    if (FAILED(hr)) {
        return false;
    }

    hr = compDisabled.GetValue(&disabled);
    HRESULT_CHECK(hr, L"compDisabled.GetValue failed");
    if (SUCCEEDED(hr) && disabled) {
        return false;
    }

    return !context->IsBlocked();
}

HRESULT ContextManager::UpdateStatus(bool foreground) {
    HRESULT hr;

    if (!this->_initialized) {
        return S_FALSE;
    }

    long openclose;
    hr = _openclose->GetValue(&openclose);
    DBG_HRESULT_CHECK(hr, L"_openclose->GetValue failed");

    DBG_DPRINT(L"openclose = %ld", openclose);

    if (foreground) {
        _settings->IsDefaultEnabled(&_defaultEnabled);
        _settings->IsBackconvert(reinterpret_cast<DWORD*>(&_backconvert));

        hr = _settings->LoadTelexSettings(_config);
        if (_focus) {
            _focus->GetEngine()->SetConfig(_config);
        }
    }

    hr = _status->UpdateStatus(!!openclose, _focus ? _focus->IsBlocked() : true);
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
        _contextMap.clear();
        return hr;
    }

    CComPtr<ITfContext> context;
    while (1) {
        ULONG fetched;
        hr = contexts->Next(1, &context, &fetched);
        if (SUCCEEDED(hr) && fetched) {
            _contextMap.erase(context.p);
        } else {
            break;
        }
        context.Release();
    }
    return S_OK;
}

STDMETHODIMP ContextManager::OnSetFocus(
    __RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) {
    HRESULT hr, hrSession;

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

    if (!context) {
        _focus.Release();
        return S_OK;
    }

    if (_focus && _focus->GetContext() == context) {
        return S_OK;
    }

    hr = OnPushContext(context);
    HRESULT_CHECK_RETURN(hr, L"creating context with OnPushContext failed");
    _focus = _contextMap.at(context);

    _focus->GetEngine()->SetConfig(_config);
    hr = _focus->UpdateBlocked(&hrSession);
    if (FAILED(hr) || FAILED(hrSession)) {
        DBG_HRESULT_CHECK(hr, L"ContextManager::RequestEditSession failed");
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnPushContext(__RPC__in_opt ITfContext* pic) {
    HRESULT hr;

    DBG_DPRINT(L"context = %p", pic);

    auto it = _contextMap.lower_bound(pic);
    if (it == _contextMap.end() || _contextMap.key_comp()(pic, it->first) != 0) {
        CComPtr<Context> context;
        hr = CreateInitialize(&context, this, pic, static_cast<const Telex::TelexConfig&>(_config), _displayAtom);
        HRESULT_CHECK_RETURN(hr, "Create Context failed");
        _contextMap.insert(it, {pic, std::move(context)});
    }

    return S_OK;
}

STDMETHODIMP ContextManager::OnPopContext(__RPC__in_opt ITfContext* pic) {
    DBG_DPRINT(L"context = %p", pic);

    _contextMap.erase(pic);

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
    hr = CreateInitialize(&_enabled, threadMgr, clientid, Globals::GUID_Compartment_EnabledToggle, true, [this] {
        return OnToggle(false);
    });
    HRESULT_CHECK_RETURN(hr, L"_enabled->Initialize failed");

    hr = CreateInitialize(&_openclose, threadMgr, clientid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, false, [this] {
        return OnToggle(true);
    });
    DBG_HRESULT_CHECK(hr, L"_openclose->Initialize failed");

    // init settings compartment & listener
    hr = CreateInitialize(&_systemNotify, threadMgr, clientid, Globals::GUID_Compartment_SystemNotify, true, [this] {
        HRESULT hr = OnSettingsChange();
        DBG_HRESULT_CHECK(hr, L"OnSettingsChange failed");
        return S_OK;
    });
    DBG_HRESULT_CHECK(hr, L"_systemNotify->Initialize failed");

    OnSettingsChange();

    // must enable self before we get focus events from the threadmgr event sink
    _initialized = true;

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"_threadMgrEventSinkAdvisor.Advise failed");

    CComPtr<ITfKeystrokeMgr> keystrokeMgr;
    hr = _threadMgr->QueryInterface(&keystrokeMgr);
    if (SUCCEEDED(hr)) {
        hr = keystrokeMgr->AdviseKeyEventSink(_clientid, this, TRUE);
        HRESULT_CHECK_RETURN(hr, L"_keystrokeMgr->AdviseKeyEventSink failed");

        _settings->GetPreservedKeyToggle(&_pk_toggle);
        hr = keystrokeMgr->PreserveKey(_clientid, Globals::GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle, NULL, 0);
        // probably not fatal
        DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->PreserveKey failed");
    } else {
        DBG_HRESULT_CHECK(hr, L"_threadMgr->QueryInterface failed");
    }

    return S_OK;
}

HRESULT ContextManager::Uninitialize() {
    HRESULT hr;

    _initialized = false;

    _contextMap.clear();

    if (_threadMgr) {
        CComPtr<ITfKeystrokeMgr> keystrokeMgr;
        hr = _threadMgr->QueryInterface(&keystrokeMgr);
        if (SUCCEEDED(hr)) {
            hr = keystrokeMgr->UnpreserveKey(Globals::GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle);
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
