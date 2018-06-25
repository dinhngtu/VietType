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
#include "EngineState.h"
#include "LanguageBar.h"

namespace VietType {

class EngineController :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ILanguageBarCallbacks,
    public ITfCompartmentEventSink {
public:
    EngineController();
    ~EngineController();

    DECLARE_NOT_AGGREGATABLE(EngineController)
    BEGIN_COM_MAP(EngineController)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ILanguageBarCallbacks
    virtual HRESULT OnClick(TfLBIClick click) override;
    virtual HRESULT InitMenu(ITfMenu * menu) override;
    virtual HRESULT OnMenuSelect(UINT id) override;

    // Inherited via ITfCompartmentEventSink
    virtual STDMETHODIMP OnChange(REFGUID rguid) override;

    HRESULT Initialize(
        std::shared_ptr<EngineState> const & engine,
        SmartComObjPtr<LanguageBarButton> const & languageBar,
        ITfThreadMgr * threadMgr,
        TfClientId clientid);
    HRESULT Uninitialize();

    EngineState& GetEngine();
    EngineState const& GetEngine() const;
    std::shared_ptr<EngineState> const& GetEngineShared();

    int IsEnabled() const;
    HRESULT WriteEnabled(int enabled);
    HRESULT ToggleEnabled();
    // update engine and langbar enabled state to match compartment value
    HRESULT UpdateEnabled();

private:
    HRESULT CompartmentReadEnabled(int *pEnabled);
    HRESULT CompartmentWriteEnabled(int enabled);

private:
    std::shared_ptr<EngineState> _engine;
    SmartComObjPtr<LanguageBarButton> _languageBar;

    TfClientId _clientid;

    SmartComPtr<ITfCompartment> _compartment;
    SinkAdvisor<ITfCompartmentEventSink> _compartmentEventSink;

private:
    DISALLOW_COPY_AND_ASSIGN(EngineController);
};

}
