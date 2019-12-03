// Copyright (c) 2018 Dinh Ngoc Tu.
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

// Derived from Microsoft's SampleIME source code included in the Windows classic samples,
// whose original copyright and permission notice is included below:
//
//     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//     ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//     THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//     PARTICULAR PURPOSE.
//
//     Copyright (c) Microsoft Corporation. All rights reserved

#include "ThreadMgrEventSink.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "TextEditSink.h"
#include "EditSessions.h"
#include "Utilities.h"

namespace VietType {

STDMETHODIMP ThreadMgrEventSink::OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnSetFocus(__RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) {
    HRESULT hr;

    DBG_DPRINT(L"pdimFocus = %p", pdimFocus);

    if (!pdimFocus) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    _docMgrFocus = pdimFocus;

    CComPtr<ITfContext> context;
    hr = _docMgrFocus->GetTop(&context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"pdimFocus->GetTop failed");

    if (!context) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    hr = OnNewContext(context, _compositionManager, _controller);
    HRESULT_CHECK_RETURN(hr, L"%s", L"OnNewContext failed");

    return S_OK;
}

STDMETHODIMP ThreadMgrEventSink::OnPushContext(__RPC__in_opt ITfContext* pic) {
    return E_NOTIMPL;
}

STDMETHODIMP ThreadMgrEventSink::OnPopContext(__RPC__in_opt ITfContext* pic) {
    return E_NOTIMPL;
}

_Check_return_ HRESULT ThreadMgrEventSink::Initialize(
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId tid,
    _In_ CompositionManager* compMgr,
    _In_ EngineController* controller) {

    HRESULT hr;

    _compositionManager = compMgr;
    _controller = controller;

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSinkAdvisor.Advise failed");

    CComPtr<ITfDocumentMgr> documentMgr;
    hr = threadMgr->GetFocus(&documentMgr);
    HRESULT_CHECK_RETURN(hr, L"%s", L"threadMgr->GetFocus failed");

    return hr;
}

HRESULT ThreadMgrEventSink::Uninitialize() {
    HRESULT hr;

    hr = _threadMgrEventSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_threadMgrEventSinkAdvisor.Unadvise failed");

    _controller.Release();
    _compositionManager.Release();

    _docMgrFocus.Release();

    return hr;
}

}
