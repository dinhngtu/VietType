// SPDX-License-Identifier: GPL-3.0-only

// Derived from Microsoft's SampleIME source code included in the Windows classic samples,
// whose original copyright and permission notice is included below:
//
//     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//     ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//     THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//     PARTICULAR PURPOSE.
//
//     Copyright (c) Microsoft Corporation. All rights reserved

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

static const std::tuple<GUID, std::wstring, TF_DISPLAYATTRIBUTE> ComposingAttributeData = std::make_tuple(
    // {7AB7384D-F5C6-43F9-B13C-80DCC788EE1D}
    GUID{0x7ab7384d, 0xf5c6, 0x43f9, {0xb1, 0x3c, 0x80, 0xdc, 0xc7, 0x88, 0xee, 0x1d}},
    std::wstring(L"Composing"),
    TF_DISPLAYATTRIBUTE{
        {TF_CT_NONE, 0}, // text foreground
        {TF_CT_NONE, 0}, // text background
        TF_LS_DOT,       // underline style
        FALSE,           // bold underline
        {TF_CT_NONE, 0}, // underline color
        TF_ATTR_INPUT    // attribute info
    });

STDMETHODIMP TextService::Activate(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) {
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP TextService::ActivateEx(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid, _In_ DWORD dwFlags) {
    DBG_DPRINT(
        L"h = %p, threadno = %ld, tid = %ld, flags = %lx", Globals::DllInstance, GetCurrentThreadId(), tid, dwFlags);

    HRESULT hr;

    _threadMgr = ptim;
    _clientId = tid;
    _activateFlags = dwFlags;

    _engine = std::make_shared<Telex::TelexEngine>(Telex::TelexConfig{});

    hr = CreateInstance2(&_attributeStore);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&_attributeStore) failed");

    CComPtr<DisplayAttributeInfo> composingAttrib;
    hr = CreateInitialize(
        &composingAttrib,
        std::get<0>(ComposingAttributeData),
        std::get<1>(ComposingAttributeData),
        std::get<2>(ComposingAttributeData));
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&attr1) failed");
    _attributeStore->AddAttribute(composingAttrib);

    hr = CreateInitialize(
        &_compositionManager, tid, _attributeStore->GetAttribute(0), static_cast<bool>(dwFlags & TF_TMAE_COMLESS));
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_compositionManager) failed");

    hr = CreateInitialize(&_engineController, _engine, ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_engineController) failed");

    long enabled;
    // this already sets enabled state if the compartment is empty
    hr = _engineController->IsUserEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_engineController->IsUserEnabled failed");
    DBG_DPRINT(L"init hr = %ld, enabled = %ld", hr, enabled);

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

    _attributeStore.Release();

    return S_OK;
}

STDMETHODIMP TextService::EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) {
    if (!_attributeStore) {
        return E_FAIL;
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
