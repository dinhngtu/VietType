// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

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

// TextService.cpp : Implementation of TextService

#include "stdafx.h"
#include "TextService.h"

VietType::TextService::TextService() {
}

VietType::TextService::~TextService() {
}

STDMETHODIMP VietType::TextService::Activate(ITfThreadMgr * ptim, TfClientId tid) {
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP VietType::TextService::ActivateEx(ITfThreadMgr * ptim, TfClientId tid, DWORD dwFlags) {
    HRESULT hr;

    _threadMgr = ptim;
    _clientId = tid;
    _activateFlags = dwFlags;

    Telex::TelexConfig engineconfig;
    engineconfig.oa_uy_tone1 = true;
    _engine = std::make_shared<Telex::TelexEngine>(engineconfig);

    hr = _compositionManager.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager.CreateInstance failed");
    _compositionManager->Initialize(tid);

    // we probably don't need this thread manager sink since it does nothing right now anyway
    /*
    hr = _threadMgrEventSink.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSink.CreateInstance failed");
    hr = _threadMgrEventSink->Initialize(ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSink->Initialize failed");
    */

    hr = _keyEventSink.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyEventSink.CreateInstance failed");
    hr = _keyEventSink->Initialize(ptim, tid, _compositionManager, _engine);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyEventSink->Initialize failed");

    return S_OK;
}

STDMETHODIMP VietType::TextService::Deactivate(void) {
    HRESULT hr;

    if (_keyEventSink) {
        hr = _keyEventSink->Uninitialize();
        DBG_HRESULT_CHECK(hr, L"%s", L"_keyEventSink->Uninitialize failed");
        _keyEventSink.Release();
    }

    /*
    if (_threadMgrEventSink) {
        _threadMgrEventSink->Uninitialize();
        _threadMgrEventSink.Release();
    }
    */

    _compositionManager.Release();

    return S_OK;
}
