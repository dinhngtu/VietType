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
    
    hr = _textEditSink->Initialize(pdimFocus);
    _prevFocusDocumentMgr = pdimPrevFocus;

    return S_OK;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnPushContext(ITfContext * pic) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::ThreadMgrEventSink::OnPopContext(ITfContext * pic) {
    return E_NOTIMPL;
}

HRESULT VietType::ThreadMgrEventSink::Initialize(ITfThreadMgr *threadMgr, TfClientId tid) {
    HRESULT hr;

    // create text edit sink before advising thread manager event sink
    // since text edit sink is used in thread manager event sink handler
    hr = _textEditSink.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_textEditSink.CreateInstance failed");

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSinkAdvisor.Advise failed");

    SmartComPtr<ITfDocumentMgr> documentMgr;
    hr = threadMgr->GetFocus(documentMgr.GetAddress());
    if (SUCCEEDED(hr)) {
        hr = _textEditSink->Initialize(documentMgr);
        DBG_HRESULT_CHECK(hr, L"%s", L"_textEditSink->Initialize failed");
    } else DBG_HRESULT_CHECK(hr, L"%s", L"threadMgr->GetFocus failed");

    return hr;
}

HRESULT VietType::ThreadMgrEventSink::Uninitialize() {
    HRESULT hr;

    hr = _threadMgrEventSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_threadMgrEventSinkAdvisor.Unadvise failed");

    if (_textEditSink) {
        _textEditSink->Uninitialize();
        DBG_HRESULT_CHECK(hr, L"%s", L"_textEditSink->Uninitialize failed");
        _textEditSink.Release();
    }

    _prevFocusDocumentMgr.Release();

    return hr;
}
