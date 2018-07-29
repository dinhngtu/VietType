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

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"
#include "Telex.h"

namespace VietType {

class IndicatorButton;
class LangBarButton;
class CompositionManager;
class EngineController;
class Compartment;

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

    EngineController() noexcept;
    ~EngineController();

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

    _Check_return_ HRESULT IsUserEnabled(_Out_ long* penabled);
    HRESULT WriteUserEnabled(_In_ long enabled);
    HRESULT ToggleUserEnabled();

    long IsEnabled() const;
    BlockedKind GetBlocked() const;
    void SetBlocked(_In_ BlockedKind blocked);
    void SetEditBlockedPending();
    bool SetEditBlockedPending(_In_ HRESULT result);
    bool IsEditBlockedPending() const;
    bool ResetEditBlockedPending();

    _Check_return_ HRESULT GetOpenClose(_Out_ long* openclose);

    // update engine and langbar enabled state to match enabled/blocked value
    HRESULT UpdateStates();

private:
    _Check_return_ HRESULT CompartmentReadEnabled(_Out_ long* pEnabled);
    HRESULT CompartmentWriteEnabled(_In_ long enabled);
    _Check_return_ HRESULT InitLanguageBar();
    HRESULT UninitLanguageBar();

private:
    std::shared_ptr<Telex::TelexEngine> _engine;
    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;

    TfClientId _clientid = TF_CLIENTID_NULL;

    CComPtr<Compartment> _settingsCompartment;
    SinkAdvisor<ITfCompartmentEventSink> _settingsCompartmentEventSink;

    CComPtr<Compartment> _openCloseCompartment;
    SinkAdvisor<ITfCompartmentEventSink> _openCloseCompartmentEventSink;

    // unique_ptr is not necessary but used just to break include cycle
    std::unique_ptr<IndicatorButton> _indicatorButton;
    std::unique_ptr<LangBarButton> _langBarButton;

    bool _enabled = true;
    BlockedKind _blocked = BlockedKind::Free;
    bool _editBlockedPending = false;
    bool _backconvertPending = false;

private:
    DISALLOW_COPY_AND_ASSIGN(EngineController);
};

}
