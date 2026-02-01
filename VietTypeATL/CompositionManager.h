// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "EditSession.h"
#include "SinkAdvisor.h"

namespace VietType {

class EngineController;

enum BackconvertModes : DWORD {
    BackconvertDisabled = 0,
    BackconvertOnBackspace = 1,
    BackconvertOnType = 2,
};

extern const GUID GUID_KeyEventSink_PreservedKey_Toggle;

class CompositionManager : public CComObjectRootEx<CComSingleThreadModel>,
                           public ITfThreadMgrEventSink,
                           public ITfKeyEventSink,
                           public ITfCompositionSink {
public:
    CompositionManager() = default;
    CompositionManager(const CompositionManager&) = delete;
    CompositionManager& operator=(const CompositionManager&) = delete;
    ~CompositionManager() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(CompositionManager)
    BEGIN_COM_MAP(CompositionManager)
    COM_INTERFACE_ENTRY(ITfThreadMgrEventSink)
    COM_INTERFACE_ENTRY(ITfKeyEventSink)
    COM_INTERFACE_ENTRY(ITfCompositionSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfThreadMgrEventSink
    virtual STDMETHODIMP OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) override;
    virtual STDMETHODIMP OnSetFocus(
        __RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(__RPC__in_opt ITfContext* pic) override;
    virtual STDMETHODIMP OnPopContext(__RPC__in_opt ITfContext* pic) override;

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

    // Inherited via ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(
        _In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) override;

    _Check_return_ HRESULT Initialize(
        _In_ ITfThreadMgr* threadMgr,
        _In_ TfClientId clientid,
        _In_ EngineController* controller,
        _In_opt_ ITfDisplayAttributeInfo* composingAttribute,
        _In_ bool comless);
    HRESULT Uninitialize();

    HRESULT OnNewContext(_In_opt_ ITfContext* context);

    HRESULT StartComposition(_In_ ITfContext* pContext);
    HRESULT EndComposition();
    bool IsComposing() const;

    // for use in edit sessions
    _Ret_maybenull_ ITfComposition* GetComposition() const;
    // for use in edit sessions only
    _Check_return_ HRESULT GetRange(_COM_Outptr_ ITfRange** range);
    TfClientId GetClientId() const;

    HRESULT StartCompositionNow(_In_ TfEditCookie ec, _In_ ITfContext* context);
    HRESULT EmptyCompositionText(_In_ TfEditCookie ec);
    HRESULT MoveCaretToEnd(_In_ TfEditCookie ec);
    HRESULT EndCompositionNow(_In_ TfEditCookie ec);
    HRESULT SetCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length);
    // this uses context to open a new edit session if there's no existing session
    HRESULT EnsureCompositionText(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_z_ LPCWSTR str, _In_ LONG length);

    template <typename... Args>
    _Check_return_ HRESULT RequestEditSessionEx(
        _In_ HRESULT (*callback)(
            TfEditCookie ec, CompositionManager* compositionManager, ITfContext* context, Args... args),
        _In_ DWORD flags,
        _Out_ HRESULT* hrSession,
        Args... args) {

        if (_clientid == TF_CLIENTID_NULL || !_context) {
            DBG_DPRINT(L"bad edit session request");
            return E_FAIL;
        }
        HRESULT hr;

        CComPtr<EditSession<CompositionManager*, ITfContext*, Args...>> session;
        hr = CreateInitialize(&session, callback, this, _context, args...);
        HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&session) failed");

        hr = _context->RequestEditSession(_clientid, session, flags, hrSession);
        HRESULT_CHECK_RETURN(hr, L"context->RequestEditSession failed");

        return hr;
    }

    template <typename... Args>
    HRESULT RequestEditSession(
        _In_ HRESULT (*callback)(
            TfEditCookie ec, CompositionManager* compositionManager, ITfContext* context, Args... args),
        Args... args) {

        HRESULT hr;
        HRESULT hrSession;

        hr = RequestEditSessionEx(callback, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession, args...);
        HRESULT_CHECK_RETURN(hr, L"context->RequestEditSession failed");

        return hr;
    }

private:
    static HRESULT _StartComposition(
        /* EditSession */ _In_ TfEditCookie ec,
        /* RequestEditSession */ _In_ CompositionManager* instance,
        /* RequestEditSession */ _In_ ITfContext* context);
    static HRESULT _EndComposition(
        /* EditSession */ _In_ TfEditCookie ec,
        /* RequestEditSession */ _In_ CompositionManager* instance,
        /* RequestEditSession */ _In_ ITfContext* context);

    HRESULT SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range);
    HRESULT ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range);

private:
    DWORD OnBackconvertBackspace(
        _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten, _In_ DWORD prevBackconvert);
    DWORD OnBackconvertRetype(
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _Out_ BOOL* pfEaten,
        _In_ DWORD prevBackconvert,
        _Out_ wchar_t* acceptedChar);
    HRESULT OnKeyDownCommon(
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _Out_ BOOL* pfEaten,
        _Out_ DWORD* isBackconvert,
        _Out_ wchar_t* acceptedChar);

    HRESULT CallKeyEditBackspace(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);
    HRESULT CallKeyEditRetype(
        _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState, _In_ wchar_t push);
    HRESULT CallKeyEdit(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState);

private:
    // sticky
    TfClientId _clientid = TF_CLIENTID_NULL;
    CComPtr<EngineController> _controller;
    CComPtr<ITfDisplayAttributeInfo> _composingAttribute;
    CComPtr<ITfThreadMgr> _threadMgr;

    // per instance
    SinkAdvisor<ITfThreadMgrEventSink> _threadMgrEventSinkAdvisor;
    TF_PRESERVEDKEY _pk_toggle;
    CComPtr<ITfCategoryMgr> _categoryMgr;

    // context
    CComPtr<ITfContext> _context;

    // per context
    CComPtr<ITfComposition> _composition;

    // shared key state buffer; for temporary use only
    BYTE _keyState[256] = {0};
};

} // namespace VietType
