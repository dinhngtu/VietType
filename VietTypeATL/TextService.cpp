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
    DBG_DPRINT(L"h = %p, threadno = %ld, tid = %ld, flags = %lx", Globals::dllInstance, GetCurrentThreadId(), tid, dwFlags);

    HRESULT hr;

    _threadMgr = ptim;
    _clientId = tid;
    _activateFlags = dwFlags;

    Telex::TelexConfig engineconfig;
    engineconfig.oa_uy_tone1 = true;
    _engine = std::make_shared<EngineState>(engineconfig);

    hr = _compositionManager.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager.CreateInstance failed");
    _compositionManager->Initialize(tid);

    hr = _engineController.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController.CreateInstance failed");
    hr = _engineController->Initialize(_engine, ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->Initialize failed");

    int enabled;
    hr = _engineController->IsUserEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->IsUserEnabled failed");
    hr = _engineController->WriteUserEnabled(enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->UpdateEnabled failed");

    hr = _keyEventSink.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyEventSink.CreateInstance failed");
    hr = _keyEventSink->Initialize(ptim, tid, _compositionManager, _engineController);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_keyEventSink->Initialize failed");

    hr = _threadMgrEventSink.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSink.CreateInstance failed");
    hr = _threadMgrEventSink->Initialize(ptim, tid, _compositionManager, _engineController);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_threadMgrEventSink->Initialize failed");

    return S_OK;
}

STDMETHODIMP VietType::TextService::Deactivate(void) {
    DBG_DPRINT(L"h = %p, threadno = %ld, tid = %ld", Globals::dllInstance, GetCurrentThreadId(), _clientId);

    HRESULT hr;

    hr = _threadMgrEventSink->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_threadMgrEventSink->Uninitialize failed");
    _threadMgrEventSink.Release();

    hr = _keyEventSink->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_keyEventSink->Uninitialize failed");
    _keyEventSink.Release();

    hr = _engineController->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"%s", L"_engineController->Uninitialize failed");
    _engineController.Release();

    _compositionManager.Release();

    return S_OK;
}
