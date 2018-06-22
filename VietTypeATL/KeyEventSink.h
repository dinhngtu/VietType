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
#include "CompositionManager.h"
#include "Telex.h"

namespace VietType {

class KeyEventSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfKeyEventSink,
    public ITfCompartmentEventSink {
public:
    KeyEventSink();
    ~KeyEventSink();

    DECLARE_NOT_AGGREGATABLE(KeyEventSink)
    BEGIN_COM_MAP(KeyEventSink)
        COM_INTERFACE_ENTRY(ITfKeyEventSink)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(ITfContext * pic, REFGUID rguid, BOOL * pfEaten) override;

    // Inherited via ITfCompartmentEventSink
    virtual STDMETHODIMP OnChange(REFGUID rguid) override;

    HRESULT Initialize(ITfThreadMgr * threadMgr, TfClientId clientid, SmartComObjPtr<CompositionManager> const& compositionManager, std::shared_ptr<Telex::TelexEngine> const& engine);
    HRESULT Uninitialize();

private:
    HRESULT ReadEnabled(int *pEnabled);
    HRESULT WriteEnabled(int enabled);
    HRESULT CallKeyEdit(ITfContext *context, WPARAM wParam, LPARAM lParam, BYTE const *keyState);

private:
    TfClientId _clientid;
    SmartComPtr<ITfKeystrokeMgr> _keystrokeMgr;
    SmartComObjPtr<CompositionManager> _compositionManager;
    std::shared_ptr<Telex::TelexEngine> _engine;

    SmartComPtr<ITfCompartment> _compartment;
    SinkAdvisor<ITfCompartmentEventSink> _compartmentEventSink;

    int _enabled = 1;

    // shared key state buffer; for temporary use only
    BYTE _keyState[256];

private:
    DISALLOW_COPY_AND_ASSIGN(KeyEventSink);
};

}
