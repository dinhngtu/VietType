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
    CompositionManager();
    ~CompositionManager();

    DECLARE_NOT_AGGREGATABLE(CompositionManager)
    BEGIN_COM_MAP(CompositionManager)
        COM_INTERFACE_ENTRY(ITfCompositionSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfCompositionSink
    virtual HRESULT OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition * pComposition) override;

    void Initialize(TfClientId clientid);

    HRESULT RequestEditSession(ITfEditSession *session);
    HRESULT StartComposition(ITfContext *pContext);
    HRESULT EndComposition();
    bool IsComposing() const;

    HRESULT EmptyCompositionText(TfEditCookie ec);
    HRESULT MoveCaretToEnd(TfEditCookie ec);
    HRESULT EndCompositionNow(TfEditCookie ec);
    HRESULT SetCompositionText(TfEditCookie ec, WCHAR const *str, LONG length);

    template <typename... Args>
    static HRESULT RequestEditSession(
        HRESULT(*callback)(TfEditCookie ec, CompositionManager *compositionManager, ITfContext *context, Args... args),
        CompositionManager *compositionManager,
        // required to call RequestEditSession
        ITfContext *context,
        Args... args) {

        assert(compositionManager->_clientid != TF_CLIENTID_NULL);
        HRESULT hr;

        SmartComObjPtr<EditSession<CompositionManager *, ITfContext *, Args...>> session;
        hr = session.CreateInstance();
        HRESULT_CHECK_RETURN(hr, L"%s", L"es.CreateInstance failed");

        session->Initialize(callback, compositionManager, context, args...);
        HRESULT hrSession;
        hr = context->RequestEditSession(compositionManager->_clientid, session, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        HRESULT_CHECK_RETURN(hr, L"%s", L"context->RequestEditSession failed");

        return S_OK;
    }

private:
    static HRESULT _StartComposition(
        /* EditSession */ TfEditCookie ec,
        /* RequestEditSession */ CompositionManager *instance,
        /* RequestEditSession */ ITfContext *context);
    static HRESULT _EndComposition(
        /* EditSession */ TfEditCookie ec,
        /* RequestEditSession */ CompositionManager *instance,
        /* RequestEditSession */ ITfContext *context);

private:
    TfClientId _clientid = TF_CLIENTID_NULL;
    SmartComPtr<ITfContext> _context;
    SmartComPtr<ITfComposition> _composition;

private:
    DISALLOW_COPY_AND_ASSIGN(CompositionManager);
};

}
