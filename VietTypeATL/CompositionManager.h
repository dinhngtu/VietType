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
#include "EditSession.h"

namespace VietType {

class CompositionManager :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfCompositionSink {
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
    virtual STDMETHODIMP OnCompositionTerminated(_In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) override;

    _Check_return_ HRESULT Initialize(_In_ TfClientId clientid, _In_ ITfDisplayAttributeInfo* composingAttribute, _In_ bool comless);
    void Uninitialize();

    HRESULT RequestEditSession(_In_ ITfEditSession* session, _In_ ITfContext* context);
    HRESULT StartComposition(_In_ ITfContext* pContext);
    HRESULT EndComposition();
    bool IsComposing() const;

    // for use in edit sessions
    _Ret_maybenull_ ITfComposition* GetComposition() const;
    // for use in edit sessions only
    _Check_return_ HRESULT GetRange(_Outptr_ ITfRange** range);
    TfClientId GetClientId() const;

    HRESULT StartCompositionNow(_In_ TfEditCookie ec, _In_ ITfContext* context);
    HRESULT EmptyCompositionText(_In_ TfEditCookie ec);
    HRESULT MoveCaretToEnd(_In_ TfEditCookie ec);
    HRESULT EndCompositionNow(_In_ TfEditCookie ec);
    HRESULT SetCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length);
    // this uses context to open a new edit session if there's no existing session
    HRESULT EnsureCompositionText(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_z_ LPCWSTR str, _In_ LONG length);

    template <typename... Args>
    static HRESULT RequestEditSession(
        _In_ HRESULT(*callback)(TfEditCookie ec, CompositionManager* compositionManager, ITfContext* context, Args... args),
        _In_ CompositionManager* compositionManager,
        // required to call RequestEditSession
        _In_ ITfContext* context,
        Args... args) {

        assert(compositionManager->_clientid != TF_CLIENTID_NULL);
        assert(context);
        HRESULT hr;

        CComPtr<EditSession<CompositionManager*, ITfContext*, Args...>> session;
        hr = CreateInstance2(&session);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&session) failed");

        session->Initialize(callback, compositionManager, context, args...);
        HRESULT hrSession;
        hr = context->RequestEditSession(compositionManager->_clientid, session, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        HRESULT_CHECK_RETURN(hr, L"%s", L"context->RequestEditSession failed");

        return S_OK;
    }

    template <typename... Args>
    static _Check_return_ HRESULT RequestEditSessionEx(
        _In_ HRESULT(*callback)(TfEditCookie ec, CompositionManager* compositionManager, ITfContext* context, Args... args),
        _In_ CompositionManager* compositionManager,
        // required to call RequestEditSession
        _In_ ITfContext* context,
        _In_ DWORD flags,
        _Out_ HRESULT* hrSession,
        Args... args) {

        assert(compositionManager->_clientid != TF_CLIENTID_NULL);
        assert(context);
        HRESULT hr;

        CComPtr<EditSession<CompositionManager*, ITfContext*, Args...>> session;
        hr = CreateInstance2(&session);
        HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&session) failed");

        session->Initialize(callback, compositionManager, context, args...);
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

    HRESULT SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range, _In_ ITfDisplayAttributeInfo* attr);
    HRESULT ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range);

private:
    TfClientId _clientid = TF_CLIENTID_NULL;
    CComPtr<ITfContext> _context;
    CComPtr<ITfComposition> _composition;
    CComPtr<ITfCategoryMgr> _categoryMgr;
    CComPtr<ITfDisplayAttributeInfo> _composingAttribute;
};

}
