// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// TextService.cpp : Implementation of TextService

#include "stdafx.h"
#include "TextService.h"
#include "CompositionManager.h"
#include "Context.h"
#include "EngineSettingsController.h"
#include "StatusController.h"
#include "EnumDisplayAttributeInfo.h"

namespace VietType {

STDMETHODIMP TextService::Activate(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) {
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP TextService::ActivateEx(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid, _In_ DWORD dwFlags) {
    DBG_DPRINT(
        L"h = %p, threadno = %ld, tid = %ld, flags = %lx", Globals::DllInstance, GetCurrentThreadId(), tid, dwFlags);

    HRESULT hr;

    hr = CreateInstance2(&_attributeStore);
    HRESULT_CHECK_RETURN(hr, L"CreateInstance2(&_attributeStore) failed");

    hr = CreateInitialize(
        &_compositionManager, ptim, tid, _attributeStore, static_cast<bool>(dwFlags & TF_TMAE_COMLESS));
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&_compositionManager) failed");

    return S_OK;
}

STDMETHODIMP TextService::Deactivate(void) {
    DBG_DPRINT(L"h = %p, threadno = %ld", Globals::DllInstance, GetCurrentThreadId());

    if (_compositionManager)
        _compositionManager->Uninitialize();
    _compositionManager.Release();

    _attributeStore.Release();

    return S_OK;
}

STDMETHODIMP TextService::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) {
    *ppEnum = nullptr;
    if (!_attributeStore) {
        return E_INVALIDARG;
    }
    *ppEnum = _attributeStore;
    (*ppEnum)->AddRef();
    return S_OK;
}

STDMETHODIMP TextService::GetDisplayAttributeInfo(
    __RPC__in REFGUID guid, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo) {
    return _attributeStore->FindAttributeByGuid(guid, ppInfo);
}

} // namespace VietType
