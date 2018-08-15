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
class TextEditSink;

class ThreadMgrEventSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfThreadMgrEventSink {
public:
    ThreadMgrEventSink() = default;
    ThreadMgrEventSink(const ThreadMgrEventSink&) = delete;
    ThreadMgrEventSink& operator=(const ThreadMgrEventSink&) = delete;
    ~ThreadMgrEventSink() = default;

    DECLARE_NOT_AGGREGATABLE(ThreadMgrEventSink)
    BEGIN_COM_MAP(ThreadMgrEventSink)
        COM_INTERFACE_ENTRY(ITfThreadMgrEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    // Inherited via ITfThreadMgrEventSink
    virtual STDMETHODIMP OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) override;
    virtual STDMETHODIMP OnSetFocus(__RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(__RPC__in_opt ITfContext* pic) override;
    virtual STDMETHODIMP OnPopContext(__RPC__in_opt ITfContext* pic) override;

    _Check_return_ HRESULT Initialize(
        _In_ ITfThreadMgr* threadMgr,
        _In_ TfClientId tid,
        _In_ CompositionManager* compMgr,
        _In_ EngineController* controller);
    HRESULT Uninitialize();

private:
    SinkAdvisor<ITfThreadMgrEventSink> _threadMgrEventSinkAdvisor;
    CComPtr<CompositionManager> _compositionManager;
    CComPtr<EngineController> _controller;
    CComPtr<ITfDocumentMgr> _docMgrFocus;
};

}
