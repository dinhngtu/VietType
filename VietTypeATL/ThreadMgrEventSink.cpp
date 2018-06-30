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

VietType::ThreadMgrEventSink::ThreadMgrEventSink() {
}

VietType::ThreadMgrEventSink::~ThreadMgrEventSink() {
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnInitDocumentMgr(ITfDocumentMgr * pdim) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnUninitDocumentMgr(ITfDocumentMgr * pdim) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnSetFocus(ITfDocumentMgr * pdimFocus, ITfDocumentMgr * pdimPrevFocus) {
    HRESULT hr;

    if (!pdimFocus || _controller->IsEditBlockedPending()) {
        return S_OK;
    }

    SmartComPtr<ITfContext> context;
    hr = pdimFocus->GetTop(context.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"pdimFocus->GetTop failed");

    if (!context) {
        return S_OK;
    }

    hr = _controller->RequestEditBlocked(_compMgr, context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_controller->RequestEditBlocked failed");

    return S_OK;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnPushContext(ITfContext * pic) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnPopContext(ITfContext * pic) {
    return E_NOTIMPL;
}

HRESULT VietType::ThreadMgrEventSink::Initialize(
    ITfThreadMgr *threadMgr,
    TfClientId tid,
    SmartComObjPtr<CompositionManager> const& compMgr,
    SmartComObjPtr<EngineController> const& controller) {

    HRESULT hr;

    _compMgr = compMgr;
    _controller = controller;

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSinkAdvisor.Advise failed");

    return hr;
}

HRESULT VietType::ThreadMgrEventSink::Uninitialize() {
    HRESULT hr;

    hr = _threadMgrEventSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_threadMgrEventSinkAdvisor.Unadvise failed");

    _controller.Release();
    _compMgr.Release();

    return hr;
}
