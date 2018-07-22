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

namespace VietType {

class CompositionManager;
class EngineController;

class KeyEventSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfKeyEventSink {
public:
    KeyEventSink();
    ~KeyEventSink();

    DECLARE_NOT_AGGREGATABLE(KeyEventSink)
    BEGIN_COM_MAP(KeyEventSink)
        COM_INTERFACE_ENTRY(ITfKeyEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfKeyEventSink
    virtual STDMETHODIMP OnSetFocus(BOOL fForeground) override;
    virtual STDMETHODIMP OnTestKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnTestKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnKeyDown(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnKeyUp(ITfContext * pic, WPARAM wParam, LPARAM lParam, BOOL * pfEaten) override;
    virtual STDMETHODIMP OnPreservedKey(ITfContext * pic, REFGUID rguid, BOOL * pfEaten) override;

    HRESULT Initialize(
        ITfThreadMgr * threadMgr,
        TfClientId clientid,
        CompositionManager* compositionManager,
        EngineController* controller);
    HRESULT Uninitialize();

private:
    HRESULT CallKeyEdit(ITfContext *context, WPARAM wParam, LPARAM lParam, BYTE const *keyState);

private:
    TfClientId _clientid;
    CComPtr<ITfKeystrokeMgr> _keystrokeMgr;
    CComPtr<ITfThreadMgr> _threadMgr;
    CComPtr<CompositionManager> _compositionManager;
    CComPtr<EngineController> _controller;

    // shared key state buffer; for temporary use only
    BYTE _keyState[256];

private:
    DISALLOW_COPY_AND_ASSIGN(KeyEventSink);
};

}
