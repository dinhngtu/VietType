// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"

namespace VietType {

class CompositionManager;
class EngineController;
class TextEditSink;

class ThreadMgrEventSink : public CComObjectRootEx<CComSingleThreadModel>, public ITfThreadMgrEventSink {
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
    virtual STDMETHODIMP OnSetFocus(
        __RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) override;
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

} // namespace VietType
