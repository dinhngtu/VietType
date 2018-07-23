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

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "ThreadMgrEventSink.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "TextEditSink.h"
#include "EditSessions.h"
#include "Utilities.h"

VietType::ThreadMgrEventSink::ThreadMgrEventSink() noexcept {
}

VietType::ThreadMgrEventSink::~ThreadMgrEventSink() {
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnSetFocus(__RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) {
    HRESULT hr;

    DBG_DPRINT(L"pdimFocus = %p, pending = %d", pdimFocus, _controller->IsEditBlockedPending());

    if (!pdimFocus) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    _docMgrFocus = pdimFocus;

    if (_controller->IsEditBlockedPending()) {
        return S_OK;
    }

    CComPtr<ITfContext> context;
    hr = _docMgrFocus->GetTop(&context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"pdimFocus->GetTop failed");

    if (!context) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    bool isempty;
    hr = VietType::IsContextEmpty(context, _compMgr->GetClientId(), &isempty);
    HRESULT_CHECK_RETURN(hr, L"%s", L"VietType::IsContextEmpty failed");
    if (isempty) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    TF_STATUS st;
    hr = context->GetStatus(&st);
    if (SUCCEEDED(hr)) {
        DBG_DPRINT(
            L"d=%c%c%c s=%c%c%c",
            (st.dwDynamicFlags & TF_SD_LOADING) ? L'L' : L'_',
            (st.dwDynamicFlags & TF_SD_READONLY) ? L'R' : L'_',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'U' : L'_',
            (st.dwStaticFlags & TF_SS_DISJOINTSEL) ? L'D' : L'_',
            (st.dwStaticFlags & TF_SS_REGIONS) ? L'R' : L'_',
            (st.dwStaticFlags & TF_SS_TRANSITORY) ? L'T' : L'_');
    } else DBG_HRESULT_CHECK(hr, L"%s", L"context->GetStatus failed");

    if (!_controller->IsEditBlockedPending()) {
        _controller->SetEditBlockedPending();
        hr = CompositionManager::RequestEditSession(
            VietType::EditSessions::EditBlocked,
            _compMgr,
            context,
            static_cast<EngineController*>(_controller));
        _controller->ResetEditBlockedPending();
        if (FAILED(hr)) {
            DBG_HRESULT_CHECK(hr, L"%s", L"CompositionManager::RequestEditSession failed");
            _controller->SetBlocked(EngineController::BlockedKind::Free);
        }
    }

    return S_OK;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnPushContext(__RPC__in_opt ITfContext* pic) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnPopContext(__RPC__in_opt ITfContext* pic) {
    return E_NOTIMPL;
}

_Check_return_ HRESULT VietType::ThreadMgrEventSink::Initialize(
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId tid,
    _In_ VietType::CompositionManager* compMgr,
    _In_ VietType::EngineController* controller) {

    HRESULT hr;

    _compMgr = compMgr;
    _controller = controller;

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSinkAdvisor.Advise failed");

    CComPtr<ITfDocumentMgr> documentMgr;
    hr = threadMgr->GetFocus(&documentMgr);
    HRESULT_CHECK_RETURN(hr, L"%s", L"threadMgr->GetFocus failed");

    return hr;
}

HRESULT VietType::ThreadMgrEventSink::Uninitialize() {
    HRESULT hr;

    hr = _threadMgrEventSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_threadMgrEventSinkAdvisor.Unadvise failed");

    _controller.Release();
    _compMgr.Release();

    _docMgrFocus.Release();

    return hr;
}
