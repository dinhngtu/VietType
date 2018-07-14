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

enum class BlockedKind {
    // don't change enable setting
    FREE,
    // don't change enable setting, but also completely block the engine
    BLOCKED,
    // change enable setting to disabled; automatically restore enable setting once free (or if overridden by user)
    ADVISED,
};

class EngineController :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfCompartmentEventSink {
public:
    EngineController();
    ~EngineController();

    DECLARE_NOT_AGGREGATABLE(EngineController)
    BEGIN_COM_MAP(EngineController)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfCompartmentEventSink
    virtual STDMETHODIMP OnChange(REFGUID rguid) override;

    HRESULT Initialize(
        std::shared_ptr<Telex::TelexEngine> const& engine,
        ITfThreadMgr * threadMgr,
        TfClientId clientid);
    HRESULT Uninitialize();

    Telex::TelexEngine& GetEngine();
    Telex::TelexEngine const& GetEngine() const;
    std::shared_ptr<Telex::TelexEngine> const& GetEngineShared();

    HRESULT IsUserEnabled(int *penabled);
    HRESULT WriteUserEnabled(int enabled);
    HRESULT ToggleUserEnabled();

    int IsEnabled() const;
    BlockedKind GetBlocked() const;
    void SetBlocked(BlockedKind blocked);
    bool ResetBlocked(HRESULT result);
    bool IsEditBlockedPending() const;

    // update engine and langbar enabled state to match enabled/blocked value
    HRESULT UpdateStates();

private:
    HRESULT CompartmentReadEnabled(int *pEnabled);
    HRESULT CompartmentWriteEnabled(int enabled);
    HRESULT InitLanguageBar();
    HRESULT UninitLanguageBar();

private:
    std::shared_ptr<Telex::TelexEngine> _engine;
    SmartComPtr<ITfLangBarItemMgr> _langBarItemMgr;

    TfClientId _clientid;

    SmartComPtr<ITfCompartment> _compartment;
    SinkAdvisor<ITfCompartmentEventSink> _compartmentEventSink;

    // unique_ptr is not necessary but used just to break include cycle
    std::unique_ptr<IndicatorButton> _indicatorButton;
    std::unique_ptr<LangBarButton> _langBarButton;

    bool _enabled = true;
    BlockedKind _blocked = BlockedKind::FREE;
    bool _editBlockedPending = false;
    bool _backconvertPending = false;

private:
    DISALLOW_COPY_AND_ASSIGN(EngineController);
};

}
