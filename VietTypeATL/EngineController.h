// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"
#include "Telex.h"
#include "Compartment.h"
#include "SettingsStore.h"
#include "LanguageBarButton.h"

namespace VietType {

class RefreshableButton;
class CompositionManager;
class EngineController;
class SettingsDialog;
class EngineSettingsController;

class EngineController : public CComObjectRootEx<CComSingleThreadModel> {
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

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(EngineController)
    BEGIN_COM_MAP(EngineController)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    _Check_return_ HRESULT
    Initialize(_In_ Telex::TelexEngine* engine, _In_ ITfThreadMgr* threadMgr, _In_ TfClientId clientid);
    HRESULT Uninitialize();

    Telex::TelexEngine& GetEngine();
    const Telex::TelexEngine& GetEngine() const;

    DWORD IsBackconvertOnBackspace();

    _Check_return_ HRESULT IsUserEnabled(_Out_ long* penabled) const;
    HRESULT ToggleUserEnabled();

    // effective enabled status, from the combination of user setting and blocked state
    long IsEnabled() const;
    BlockedKind GetBlocked() const;
    void SetBlocked(_In_ BlockedKind blocked);

    EngineSettingsController* GetSettings() const;

    // update engine and langbar enabled state to match enabled/blocked value
    HRESULT UpdateStates(bool foreground);

private:
    _Check_return_ HRESULT InitLanguageBar();
    HRESULT UninitLanguageBar();

private:
    bool _initialized = false;

    Telex::TelexEngine* _engine = nullptr;
    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;

    TfClientId _clientid = TF_CLIENTID_NULL;

    CComPtr<CachedCompartmentSetting<long>> _enabled;

    CComPtr<LanguageBarButton> _indicatorButton;
    CComPtr<LanguageBarButton> _langBarButton;

    CComPtr<EngineSettingsController> _settings;
    // cached settings
    DWORD _defaultEnabled = 0;
    DWORD _backconvertOnBackspace = 0;
    CComPtr<CompartmentNotifier> _systemNotify;

    BlockedKind _blocked = BlockedKind::Free;
};

} // namespace VietType
