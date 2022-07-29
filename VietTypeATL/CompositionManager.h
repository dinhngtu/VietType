// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "EditSession.h"

namespace VietType {

class CompositionManager : public CComObjectRootEx<CComSingleThreadModel>, public ITfCompositionSink {
public:
    CompositionManager() = default;
    CompositionManager(const CompositionManager&) = delete;
    CompositionManager& operator=(const CompositionManager&) = delete;
    ~CompositionManager() = default;

    DECLARE_NOT_AGGREGATABLE(CompositionManager)
    BEGIN_COM_MAP(CompositionManager)
    COM_INTERFACE_ENTRY(ITfCompositionSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfCompositionSink
    virtual STDMETHODIMP OnCompositionTerminated(
        _In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) override;

    _Check_return_ HRESULT
    Initialize(_In_ TfClientId clientid, _In_ ITfDisplayAttributeInfo* composingAttribute, _In_ bool comless);
    HRESULT Uninitialize();

    HRESULT RequestEditSession(
        _In_ ITfEditSession* session,
        _In_ ITfContext* context,
        _In_ DWORD flags = TF_ES_ASYNCDONTCARE | TF_ES_READWRITE);
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
    HRESULT SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range) {
        return this->SetRangeDisplayAttribute(ec, context, range, _composingAttribute);
    }

    template <typename... Args>
    static HRESULT RequestEditSession(
        _In_ HRESULT (*callback)(
            TfEditCookie ec, CompositionManager* compositionManager, ITfContext* context, Args... args),
        _In_ CompositionManager* compositionManager,
        // required to call RequestEditSession
        _In_ ITfContext* context,
        Args... args) {

        if (compositionManager->_clientid == TF_CLIENTID_NULL || !context) {
            DBG_DPRINT(L"%s", L"bad edit session request");
            return E_FAIL;
        }
        HRESULT hr;

        CComPtr<EditSession<CompositionManager*, ITfContext*, Args...>> session;
        hr = CreateInitialize(&session, callback, compositionManager, context, args...);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&session) failed");

        HRESULT hrSession;
        hr = context->RequestEditSession(
            compositionManager->_clientid, session, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        HRESULT_CHECK_RETURN(hr, L"%s", L"context->RequestEditSession failed");

        return S_OK;
    }

    template <typename... Args>
    static _Check_return_ HRESULT RequestEditSessionEx(
        _In_ HRESULT (*callback)(
            TfEditCookie ec, CompositionManager* compositionManager, ITfContext* context, Args... args),
        _In_ CompositionManager* compositionManager,
        // required to call RequestEditSession
        _In_ ITfContext* context,
        _In_ DWORD flags,
        _Out_ HRESULT* hrSession,
        Args... args) {

        if (compositionManager->_clientid == TF_CLIENTID_NULL || !context) {
            DBG_DPRINT(L"%s", L"bad edit session request");
            return E_FAIL;
        }
        HRESULT hr;

        CComPtr<EditSession<CompositionManager*, ITfContext*, Args...>> session;
        hr = CreateInitialize(&session, callback, compositionManager, context, args...);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&session) failed");

        hr = context->RequestEditSession(compositionManager->_clientid, session, flags, hrSession);
        HRESULT_CHECK_RETURN(hr, L"%s", L"context->RequestEditSession failed");

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

    HRESULT SetRangeDisplayAttribute(
        _In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range, _In_ ITfDisplayAttributeInfo* attr);
    HRESULT ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range);

private:
    TfClientId _clientid = TF_CLIENTID_NULL;
    CComPtr<ITfContext> _context;
    CComPtr<ITfComposition> _composition;
    CComPtr<ITfCategoryMgr> _categoryMgr;
    CComPtr<ITfDisplayAttributeInfo> _composingAttribute;
};

} // namespace VietType
