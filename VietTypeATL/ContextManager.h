// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"
#include "SettingsStore.h"
#include "KeyTranslator.h"
#include "Telex.h"

namespace VietType {

class StatusController;
class EngineSettingsController;
class Context;

using ContextMap = std::map<ITfContext*, CComPtr<Context>>;

class ATL_NO_VTABLE ContextManager : public CComObjectRootEx<CComSingleThreadModel>,
                                     public ITfThreadMgrEventSink,
                                     public ITfThreadFocusSink,
                                     public ITfKeyEventSink {
public:
    ContextManager() = default;
    ContextManager(const ContextManager&) = delete;
    ContextManager& operator=(const ContextManager&) = delete;
    ~ContextManager() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(ContextManager)
    BEGIN_COM_MAP(ContextManager)
    COM_INTERFACE_ENTRY(ITfThreadMgrEventSink)
    COM_INTERFACE_ENTRY(ITfThreadFocusSink)
    COM_INTERFACE_ENTRY(ITfKeyEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfThreadMgrEventSink
    virtual STDMETHODIMP OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) override;
    virtual STDMETHODIMP OnSetFocus(
        __RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(__RPC__in_opt ITfContext* pic) override;
    virtual STDMETHODIMP OnPopContext(__RPC__in_opt ITfContext* pic) override;

    // Inherited via ITfThreadFocusSink
    virtual STDMETHODIMP OnSetThreadFocus() override;
    virtual STDMETHODIMP OnKillThreadFocus() override;

    // Inherited via ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(_In_ BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(
        _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(
        _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(
        _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(
        _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(_In_ ITfContext* pic, _In_ REFGUID rguid, _Out_ BOOL* pfEaten) override;

    _Check_return_ HRESULT Initialize(
        _In_ ITfThreadMgr* threadMgr,
        _In_ TfClientId clientid,
        _In_ EngineSettingsController* settings,
        _In_ TfGuidAtom displayAtom);
    HRESULT Uninitialize();
    void FinalRelease() {
        Uninitialize();
    }

    const Telex::TelexConfig& GetConfig() const {
        return _config;
    }
    uint64_t GetConfigVersion() const {
        return _configVersion;
    }
    EngineSettingsController* GetSettings() const {
        return _settings;
    }

    constexpr TfClientId GetClientId() const {
        return _clientid;
    }

    HRESULT ToggleUserEnabled();

private:
    bool IsEnabled(_In_ Context* context) const;

    HRESULT OnToggle(bool fromOpenClose);
    HRESULT OnSettingsChange();
    // `foreground` is only advisory as we'll update status when `foreground == false` if we have input focus
    HRESULT UpdateStatus(bool foreground);

    HRESULT OnKeyCommon(
        _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_ bool update, _Out_ BOOL* pfEaten);

    HRESULT CallKeyEditBackspace(_In_ Context* context);
    HRESULT CallKeyEditRetype(_In_ Context* context, _In_ wchar_t push);
    HRESULT CallKeyEdit(_In_ Context* context, _In_ bool sync, _In_ KeyResult keyResult, _In_ wchar_t push);

private:
    // from parent
    TfClientId _clientid = TF_CLIENTID_NULL;
    CComPtr<ITfThreadMgr> _threadMgr;
    TfGuidAtom _displayAtom = TF_INVALID_GUIDATOM;
    CComPtr<EngineSettingsController> _settings;

    // state
    CComPtr<StatusController> _status;

    Telex::TelexConfig _config{};
    uint64_t _configVersion = 0;
    CComPtr<CachedCompartmentSetting<long>> _enabled;
    CComPtr<CachedCompartmentSetting<long>> _openclose;

    // cached settings
    DWORD _defaultEnabled = 0;
    BackconvertModes _backconvert = BackconvertDisabled;

    CComPtr<CompartmentNotifier> _systemNotify;

    SinkAdvisor<ITfThreadMgrEventSink> _threadMgrEventSinkAdvisor;
    SinkAdvisor<ITfThreadFocusSink> _threadFocusSinkAdvisor;
    TF_PRESERVEDKEY _pk_toggle;

    bool _initialized = false;

    ContextMap _contextMap;
    CComPtr<Context> _focus;

    // shared key state buffer; for temporary use only
    BYTE _keyState[256] = {0};
};

} // namespace VietType
