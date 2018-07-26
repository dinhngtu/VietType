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
#include "EnumDisplayAttributeInfo.h"
#include "DisplayAttributes.h"
#include "ThreadMgrEventSink.h"
#include "KeyEventSink.h"
#include "CompositionManager.h"
#include "Telex.h"
#include "EngineController.h"

static CComPtr<VietType::EnumDisplayAttributeInfo> CreateAttributeStore() {
    HRESULT hr;

    CComPtr<VietType::EnumDisplayAttributeInfo> ret;
    hr = CreateInstance2(&ret);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"CreateInstance2(&ret) failed");
        ret = nullptr;
        return ret;
    }

    CComPtr<VietType::DisplayAttributeInfo> attr1;
    hr = ConstructInstance(
        &attr1,
        std::get<0>(VietType::ComposingAttributeData),
        std::get<1>(VietType::ComposingAttributeData),
        std::get<2>(VietType::ComposingAttributeData));
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"ConstructInstance(&attr1) failed");
        ret = nullptr;
        return ret;
    }

    CComPtr<ITfDisplayAttributeInfo> info1(attr1.p);
    assert(info1);
    ret->AddAttribute(info1);

    return ret;
}

static CComPtr<VietType::EnumDisplayAttributeInfo> attributeStore = CreateAttributeStore();

VietType::TextService::TextService() noexcept {
}

VietType::TextService::~TextService() {
}

STDMETHODIMP VietType::TextService::Activate(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) {
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP VietType::TextService::ActivateEx(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid, _In_ DWORD dwFlags) {
    DBG_DPRINT(L"h = %p, threadno = %ld, tid = %ld, flags = %lx", Globals::DllInstance, GetCurrentThreadId(), tid, dwFlags);

    HRESULT hr;

    _threadMgr = ptim;
    _clientId = tid;
    _activateFlags = dwFlags;

    Telex::TelexConfig engineconfig;
    engineconfig.oa_uy_tone1 = true;
    _engine = std::make_shared<Telex::TelexEngine>(engineconfig);

    hr = ConstructInstance(&_compositionManager, tid, attributeStore->GetAttribute(0), static_cast<bool>(dwFlags & TF_TMAE_COMLESS));
    HRESULT_CHECK_RETURN(hr, L"%s", L"ConstructInstance(&_compositionManager) failed");

    hr = ConstructInstance(&_engineController, _engine, ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"ConstructInstance(&_engineController) failed");

    long enabled;
    hr = _engineController->IsUserEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->IsUserEnabled failed");
    hr = _engineController->WriteUserEnabled(enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->UpdateEnabled failed");

    hr = ConstructInstance(&_keyEventSink, ptim, tid, _compositionManager, _engineController);
    HRESULT_CHECK_RETURN(hr, L"%s", L"ConstructInstance(&_keyEventSink) failed");

    hr = ConstructInstance(&_threadMgrEventSink, ptim, tid, _compositionManager, _engineController);
    HRESULT_CHECK_RETURN(hr, L"%s", L"ConstructInstance(&_threadMgrEventSink) failed");

    return S_OK;
}

STDMETHODIMP VietType::TextService::Deactivate(void) {
    DBG_DPRINT(L"h = %p, threadno = %ld, tid = %ld", Globals::DllInstance, GetCurrentThreadId(), _clientId);

    _threadMgrEventSink.Release();
    _keyEventSink.Release();
    _engineController.Release();
    _compositionManager.Release();
    _engine.reset();
    _threadMgr.Release();

    return S_OK;
}

STDMETHODIMP VietType::TextService::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) {
    if (!attributeStore) {
        return E_FAIL;
    }
    *ppEnum = attributeStore;
    (*ppEnum)->AddRef();
    return S_OK;
}

STDMETHODIMP VietType::TextService::GetDisplayAttributeInfo(__RPC__in REFGUID guid, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo) {
    return attributeStore->FindAttributeByGuid(guid, ppInfo);
}
