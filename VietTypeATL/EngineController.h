// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"
#include "Telex.h"
#include "Compartment.h"
#include "SettingsStore.h"

namespace VietType {

class RefreshableButton;
class CompositionManager;
class EngineController;
class SettingsDialog;
class EngineSettingsController;

class EngineController :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfCompartmentEventSink {
public:
    enum class BlockedKind {
        // don't change enable setting
        Free,
        // don't change enable setting, but also completely block the engine
        Blocked,
    };

    EngineController() = default;
    EngineController(const EngineController&) = delete;
    EngineController& operator=(const EngineController&) = delete;
    ~EngineController() = default;

    DECLARE_NOT_AGGREGATABLE(EngineController)
    BEGIN_COM_MAP(EngineController)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfCompartmentEventSink
    virtual STDMETHODIMP OnChange(__RPC__in REFGUID rguid) override;

    _Check_return_ HRESULT Initialize(
        _In_ const std::shared_ptr<Telex::TelexEngine>& engine,
        _In_ ITfThreadMgr* threadMgr,
        _In_ TfClientId clientid);
    HRESULT Uninitialize();

    Telex::TelexEngine& GetEngine();
    const Telex::TelexEngine& GetEngine() const;

    _Check_return_ HRESULT IsUserEnabled(_Out_ long* penabled) const;
    HRESULT ToggleUserEnabled();

    // effective enabled status, from the combination of user setting and blocked state
    long IsEnabled() const;
    BlockedKind GetBlocked() const;
    void SetBlocked(_In_ BlockedKind blocked);

    _Check_return_ HRESULT GetOpenClose(_Out_ long* openclose);

    SettingsDialog CreateSettingsDialog();
    HRESULT CommitSettings(const SettingsDialog& dlg);

    // update engine and langbar enabled state to match enabled/blocked value
    HRESULT UpdateStates();

private:
    _Check_return_ HRESULT InitLanguageBar();
    HRESULT UninitLanguageBar();

private:
    bool _initialized = false;

    std::shared_ptr<Telex::TelexEngine> _engine;
    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;

    TfClientId _clientid = TF_CLIENTID_NULL;

    CComPtr<CachedCompartmentSetting<long>> _enabled;

    Compartment<long> _openCloseCompartment;
    SinkAdvisor<ITfCompartmentEventSink> _openCloseCompartmentEventSink;

    // unique_ptr is not necessary but used just to break include cycle
    std::unique_ptr<RefreshableButton> _indicatorButton;
    std::unique_ptr<RefreshableButton> _langBarButton;

    CComPtr<EngineSettingsController> _settings;

    BlockedKind _blocked = BlockedKind::Free;
};

}
