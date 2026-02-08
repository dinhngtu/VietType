// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// TextService.cpp : Implementation of TextService

#include "stdafx.h"
#include "TextService.h"
#include "ContextManager.h"
#include "Context.h"
#include "EngineSettingsController.h"
#include "StatusController.h"
#include "DisplayAttributes.h"
#include "EnumDisplayAttributeInfo.h"
#include "Compartment.h"

namespace VietType {

// {7AB7384D-F5C6-43F9-B13C-80DCC788EE1D}
static const GUID GUID_ComposingAttribute = {
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

    hr = CreateInstance2(&_attributeStore);
    HRESULT_CHECK_RETURN(hr, L"CreateInstance2(&_attributeStore) failed");

    hr = CreateInitialize(&_settings, ptim, tid);
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(_settings) failed");

    hr = InitializeDisplayAttributes(ptim, tid);
    DBG_HRESULT_CHECK(hr, L"InitializeDisplayAttributes failed");

    hr = CreateInitialize(&_compositionManager, ptim, tid, _settings, _displayAtom);
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&_compositionManager) failed");

    return S_OK;
}

STDMETHODIMP TextService::Deactivate(void) {
    DBG_DPRINT(L"h = %p, threadno = %ld", Globals::DllInstance, GetCurrentThreadId());

    if (_compositionManager)
        _compositionManager->Uninitialize();
    _compositionManager.Release();

    _displayAtom = TF_INVALID_GUIDATOM;

    if (_systemNotify)
        _systemNotify->Uninitialize();
    _systemNotify.Release();

    if (_settings)
        _settings->Uninitialize();
    _settings.Release();

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

HRESULT TextService::InitializeDisplayAttributes(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) {
    HRESULT hr;

    _threadMgr = ptim;

    CComPtr<ITfDisplayAttributeInfo> composingAttrib;
    hr = UpdateDisplayAttributes(&composingAttrib);
    HRESULT_CHECK_RETURN(hr, L"UpdateDisplayAttributes failed");

    CComPtr<ITfCategoryMgr> categoryMgr;
    hr = _threadMgr->QueryInterface(&categoryMgr);
    HRESULT_CHECK_RETURN(hr, L"_threadMgr->QueryInterface failed");

    GUID guid;
    hr = composingAttrib->GetGUID(&guid);
    HRESULT_CHECK_RETURN(hr, L"attr->GetGUID failed");

    hr = categoryMgr->RegisterGUID(guid, &_displayAtom);
    HRESULT_CHECK_RETURN(hr, L"categoryMgr->RegisterGUID failed");

    hr = CreateInitialize(&_systemNotify, _threadMgr, tid, Globals::GUID_Compartment_SystemNotify, true, [this] {
        HRESULT hr = UpdateDisplayAttributes(nullptr);
        HRESULT_CHECK_RETURN(hr, L"UpdateDisplayAttributes failed");

        CComPtr<ITfDisplayAttributeMgr> attrMgr;
        hr = _threadMgr->QueryInterface(&attrMgr);
        HRESULT_CHECK_RETURN(hr, L"_threadMgr->QueryInterface failed");

        hr = attrMgr->OnUpdateInfo();
        HRESULT_CHECK_RETURN(hr, L"attrMgr->OnUpdateInfo failed");

        return S_OK;
    });
    HRESULT_CHECK_RETURN(hr, L"_systemNotify->Initialize failed");

    return S_OK;
}

HRESULT TextService::UpdateDisplayAttributes(_COM_Outptr_opt_ ITfDisplayAttributeInfo** ppInfo) {
    HRESULT hr;

    if (ppInfo)
        *ppInfo = nullptr;

    DWORD showComposingAttr;
    _settings->IsShowingComposingAttr(&showComposingAttr);
    if (showComposingAttr >= ComposingAttributes.size())
        showComposingAttr = 0;

    CComPtr<DisplayAttributeInfo> composingAttrib(static_cast<DisplayAttributeInfo*>(_attributeStore->GetAttribute(0)));
    if (composingAttrib) {
        hr = composingAttrib->SetAttributeInfo(&ComposingAttributes[showComposingAttr]);
        HRESULT_CHECK_RETURN(hr, L"SetAttributeInfo failed");

    } else {
        hr = CreateInitialize(
            &composingAttrib, GUID_ComposingAttribute, L"Composing", ComposingAttributes[showComposingAttr]);
        HRESULT_CHECK_RETURN(hr, L"CreateInstance2(&attr1) failed");
        _attributeStore->AddAttribute(composingAttrib);
    }

    if (ppInfo) {
        *ppInfo = composingAttrib;
        (*ppInfo)->AddRef();
    }
    return S_OK;
}

} // namespace VietType
