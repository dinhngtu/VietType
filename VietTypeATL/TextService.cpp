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

// Derived from Microsoft's SampleIME source code included in the Windows classic samples:
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// TextService.cpp : Implementation of TextService

#include "stdafx.h"
#include "TextService.h"
#include "EnumDisplayAttributeInfo.h"
#include "DisplayAttributes.h"
#include "ThreadMgrEventSink.h"
#include "KeyEventSink.h"
#include "CompositionManager.h"
#include "Telex.h"
#include "EngineController.h"
#include "Compartment.h"
#include "LanguageBarHandlers.h"
#include "EngineSettingsController.h"

namespace VietType {

static CComPtr<EnumDisplayAttributeInfo> CreateAttributeStore() {
    HRESULT hr;

    CComPtr<EnumDisplayAttributeInfo> ret;
    hr = CreateInstance2(&ret);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"CreateInstance2(&ret) failed");
        ret = nullptr;
        return ret;
    }

    CComPtr<DisplayAttributeInfo> attr1;
    hr = CreateInstance2(&attr1);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"CreateInstance2(&attr1) failed");
        ret = nullptr;
        return ret;
    }

    attr1->Initialize(
        std::get<0>(ComposingAttributeData),
        std::get<1>(ComposingAttributeData),
        std::get<2>(ComposingAttributeData));
    CComPtr<ITfDisplayAttributeInfo> info1(attr1.p);
    assert(info1);
    ret->AddAttribute(info1);

    return ret;
}

static CComPtr<EnumDisplayAttributeInfo> attributeStore = CreateAttributeStore();

STDMETHODIMP TextService::Activate(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) {
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP TextService::ActivateEx(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid, _In_ DWORD dwFlags) {
    DBG_DPRINT(L"h = %p, threadno = %ld, tid = %ld, flags = %lx", Globals::DllInstance, GetCurrentThreadId(), tid, dwFlags);

    HRESULT hr;

    _threadMgr = ptim;
    _clientId = tid;
    _activateFlags = dwFlags;

    _engine = std::make_shared<Telex::TelexEngine>(Telex::TelexConfig{});

    hr = CreateInitialize(&_compositionManager, tid, attributeStore->GetAttribute(0), static_cast<bool>(dwFlags & TF_TMAE_COMLESS));
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_compositionManager) failed");

    hr = CreateInitialize(&_engineController, _engine, ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_engineController) failed");

    long enabled;
    // this already sets enabled state if the compartment is empty
    hr = _engineController->IsUserEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->IsUserEnabled failed");
    DBG_DPRINT(L"hr = %ld, enabled = %ld", hr, enabled);

    hr = CreateInitialize(&_keyEventSink, ptim, tid, _compositionManager, _engineController);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_keyEventSink) failed");

    hr = CreateInitialize(&_threadMgrEventSink, ptim, tid, _compositionManager, _engineController);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_threadMgrEventSink) failed");

    return S_OK;
}

STDMETHODIMP TextService::Deactivate(void) {
    DBG_DPRINT(L"h = %p, threadno = %ld, tid = %ld", Globals::DllInstance, GetCurrentThreadId(), _clientId);

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

    _compositionManager->Uninitialize();
    _compositionManager.Release();

    return S_OK;
}

STDMETHODIMP TextService::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) {
    if (!attributeStore) {
        return E_FAIL;
    }
    *ppEnum = attributeStore;
    (*ppEnum)->AddRef();
    return S_OK;
}

STDMETHODIMP TextService::GetDisplayAttributeInfo(__RPC__in REFGUID guid, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo) {
    return attributeStore->FindAttributeByGuid(guid, ppInfo);
}

}
