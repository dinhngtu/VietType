// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// TextService.cpp : Implementation of TextService

#include "stdafx.h"
#include "TextService.h"
#include "EnumDisplayAttributeInfo.h"
#include "DisplayAttributes.h"
#include "CompositionManager.h"
#include "Telex.h"
#include "EngineController.h"
#include "Compartment.h"
#include "EngineSettingsController.h"

namespace VietType {

// {7AB7384D-F5C6-43F9-B13C-80DCC788EE1D}
static const GUID ComposingAttributeGuid = {
    0x7ab7384d, 0xf5c6, 0x43f9, {0xb1, 0x3c, 0x80, 0xdc, 0xc7, 0x88, 0xee, 0x1d}};
static const std::array<TF_DISPLAYATTRIBUTE, 2> ComposingAttributes = {
    TF_DISPLAYATTRIBUTE{
        {TF_CT_NONE, 0}, // text foreground
        {TF_CT_NONE, 0}, // text background
        TF_LS_NONE,      // underline style
        FALSE,           // bold underline
        {TF_CT_NONE, 0}, // underline color
        TF_ATTR_INPUT    // attribute info
    },
    TF_DISPLAYATTRIBUTE{
        {TF_CT_NONE, 0}, // text foreground
        {TF_CT_NONE, 0}, // text background
        TF_LS_DOT,       // underline style
        FALSE,           // bold underline
        {TF_CT_NONE, 0}, // underline color
        TF_ATTR_INPUT    // attribute info
    },
};

STDMETHODIMP TextService::Activate(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) {
    return ActivateEx(ptim, tid, 0);
}

STDMETHODIMP TextService::ActivateEx(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid, _In_ DWORD dwFlags) {
    DBG_DPRINT(
        L"h = %p, threadno = %ld, tid = %ld, flags = %lx", Globals::DllInstance, GetCurrentThreadId(), tid, dwFlags);

    HRESULT hr;

    _engine = std::unique_ptr<Telex::ITelexEngine>(Telex::TelexNew(Telex::TelexConfig{}));

    hr = CreateInitialize(&_engineController, _engine.get(), ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&_engineController) failed");

    hr = CreateInstance2(&_attributeStore);
    HRESULT_CHECK_RETURN(hr, L"CreateInstance2(&_attributeStore) failed");

    CComPtr<DisplayAttributeInfo> composingAttrib;
    DWORD showComposingAttr;
    _engineController->GetSettings()->IsShowingComposingAttr(&showComposingAttr);
    if (showComposingAttr >= ComposingAttributes.size())
        showComposingAttr = 0;
    hr = CreateInitialize(
        &composingAttrib, ComposingAttributeGuid, L"Composing", ComposingAttributes[showComposingAttr]);
    HRESULT_CHECK_RETURN(hr, L"CreateInstance2(&attr1) failed");
    _attributeStore->AddAttribute(composingAttrib);

    long enabled;
    // this already sets enabled state if the compartment is empty
    hr = _engineController->IsUserEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"_engineController->IsUserEnabled failed");
    DBG_DPRINT(L"init hr = %ld, enabled = %ld", hr, enabled);

    hr = CreateInitialize(
        &_compositionManager,
        ptim,
        tid,
        _engineController,
        static_cast<ITfDisplayAttributeInfo*>(composingAttrib),
        static_cast<bool>(dwFlags & TF_TMAE_COMLESS));
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&_compositionManager) failed");

    return S_OK;
}

STDMETHODIMP TextService::Deactivate(void) {
    DBG_DPRINT(L"h = %p, threadno = %ld", Globals::DllInstance, GetCurrentThreadId());

    HRESULT hr;

    _compositionManager->Uninitialize();
    _compositionManager.Release();

    _attributeStore.Release();

    hr = _engineController->Uninitialize();
    DBG_HRESULT_CHECK(hr, L"_engineController->Uninitialize failed");
    _engineController.Release();

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
