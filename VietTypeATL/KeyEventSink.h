// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"

namespace VietType {

class CompositionManager;
class EngineController;

class KeyEventSink : public CComObjectRootEx<CComSingleThreadModel>, public ITfKeyEventSink {
public:
    KeyEventSink() = default;
    KeyEventSink(const KeyEventSink&) = delete;
    KeyEventSink& operator=(const KeyEventSink&) = delete;
    ~KeyEventSink() = default;

    DECLARE_NOT_AGGREGATABLE(KeyEventSink)
    BEGIN_COM_MAP(KeyEventSink)
    COM_INTERFACE_ENTRY(ITfKeyEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT OnKeyDownCommon(
        _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten, _Out_ BOOL* isBackconvert);

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
        _In_ CompositionManager* compositionManager,
        _In_ EngineController* controller);
    HRESULT Uninitialize();

private:
    HRESULT CallKeyEdit(
        _In_ ITfContext* context, _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);

private:
    TfClientId _clientid = TF_CLIENTID_NULL;
    CComPtr<ITfKeystrokeMgr> _keystrokeMgr;
    CComPtr<ITfThreadMgr> _threadMgr;
    CComPtr<CompositionManager> _compositionManager;
    CComPtr<EngineController> _controller;

    // shared key state buffer; for temporary use only
    BYTE _keyState[256] = {0};
};

} // namespace VietType
