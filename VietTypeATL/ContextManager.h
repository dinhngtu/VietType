// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"
#include "SettingsStore.h"
#include "Telex.h"

namespace VietType {

class StatusController;
class EngineSettingsController;
class Context;

enum BackconvertModes : DWORD {
    BackconvertDisabled = 0,
    BackconvertOnBackspace = 1,
    BackconvertOnType = 2,
};

extern const GUID GUID_KeyEventSink_PreservedKey_Toggle;

using ContextMap = std::map<ITfContext*, CComPtr<Context>>;

class ATL_NO_VTABLE ContextManager : public CComObjectRootEx<CComSingleThreadModel>,
                                     public ITfThreadMgrEventSink,
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
    EngineSettingsController* GetSettings() const {
        return _settings;
    }

    HRESULT OnFocusContext(_In_opt_ ITfContext* context);
    HRESULT OnOpenClose();

    constexpr TfClientId GetClientId() const {
        return _clientid;
    }

    // API for context uses
    constexpr bool IsDefaultEnabled() const {
        return _defaultEnabled;
    }
    _Check_return_ HRESULT IsUserEnabled(_Out_ long* penabled) const;
    HRESULT ToggleUserEnabled();

    HRESULT UpdateStatus(_In_ Context* context);

private:
    long IsEnabled(_In_ Context* context) const;
    HRESULT UpdateStatus(bool foreground);

    DWORD OnBackconvertBackspace(
        _In_ Context* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten, _In_ DWORD prevBackconvert);
    DWORD OnBackconvertRetype(
        _In_ Context* context,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _Out_ BOOL* pfEaten,
        _In_ DWORD prevBackconvert,
        _Out_ wchar_t* acceptedChar);
    HRESULT OnKeyDownCommon(
        _In_ Context* context,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _Out_ BOOL* pfEaten,
        _Out_ DWORD* isBackconvert,
        _Out_ wchar_t* acceptedChar);

    HRESULT CallKeyEditBackspace(
        _In_ Context* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);
    HRESULT CallKeyEditRetype(
        _In_ Context* context,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _In_reads_(256) const BYTE* keyState,
        _In_ wchar_t push);
    HRESULT CallKeyEdit(
        _In_ Context* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);

private:
    // from parent
    TfClientId _clientid = TF_CLIENTID_NULL;
    CComPtr<ITfThreadMgr> _threadMgr;
    TfGuidAtom _displayAtom = TF_INVALID_GUIDATOM;
    CComPtr<EngineSettingsController> _settings;

    // state
    CComPtr<StatusController> _status;

    Telex::TelexConfig _config{};
    CComPtr<CachedCompartmentSetting<long>> _enabled;
    CComPtr<CachedCompartmentSetting<long>> _openclose;

    // cached settings
    DWORD _defaultEnabled = 0;
    DWORD _backconvert = 0;

    CComPtr<CompartmentNotifier> _systemNotify;

    SinkAdvisor<ITfThreadMgrEventSink> _threadMgrEventSinkAdvisor;
    TF_PRESERVEDKEY _pk_toggle;

    bool _initialized = false;

    ContextMap _map;
    CComPtr<Context> _focus;

    // shared key state buffer; for temporary use only
    BYTE _keyState[256] = {0};
};

} // namespace VietType
