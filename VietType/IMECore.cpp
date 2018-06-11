// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "IMECore.h"

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT IMECore::CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj) {
    IMECore *ime = nullptr;
    HRESULT hr = S_OK;

    if (ppvObj == nullptr) {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (nullptr != pUnkOuter) {
        return CLASS_E_NOAGGREGATION;
    }

    ime = new (std::nothrow) IMECore();
    if (ime == nullptr) {
        return E_OUTOFMEMORY;
    }

    hr = ime->QueryInterface(riid, ppvObj);

    ime->Release();

    return hr;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

IMECore::IMECore() {
    DllAddRef();

    //_hkl = LoadKeyboardLayout(L"00000409", 0);

    _pThreadMgr = nullptr;

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;

    _pTextEditSinkContext = nullptr;
    _textEditSinkCookie = TF_INVALID_COOKIE;

    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

    _pDocMgrLastFocused = nullptr;

    _pContext = nullptr;

    _refCount = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

IMECore::~IMECore() {
    //UnloadKeyboardLayout(_hkl);

    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI IMECore::QueryInterface(REFIID riid, _Outptr_ void **ppvObj) {
    if (ppvObj == nullptr) {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor)) {
        *ppvObj = (ITfTextInputProcessor *)this;
    } else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx)) {
        *ppvObj = (ITfTextInputProcessorEx *)this;
    } else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink)) {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    } else if (IsEqualIID(riid, IID_ITfTextEditSink)) {
        *ppvObj = (ITfTextEditSink *)this;
    } else if (IsEqualIID(riid, IID_ITfKeyEventSink)) {
        *ppvObj = (ITfKeyEventSink *)this;
    } else if (IsEqualIID(riid, IID_ITfThreadFocusSink)) {
        *ppvObj = (ITfThreadFocusSink *)this;
    } else if (IsEqualIID(riid, IID_ITfFunctionProvider)) {
        *ppvObj = (ITfFunctionProvider *)this;
    } else if (IsEqualIID(riid, IID_ITfFunction)) {
        *ppvObj = (ITfFunction *)this;
    }

    if (*ppvObj) {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) IMECore::AddRef() {
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) IMECore::Release() {
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0) {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx::ActivateEx
//
//----------------------------------------------------------------------------

STDAPI IMECore::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags) {
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;
    _dwActivateFlags = dwFlags;

    ITfDocumentMgr *pDocMgrFocus = nullptr;

    if (!_InitThreadMgrEventSink()) {
        goto ExitError;
    }

    if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocMgrFocus)) && (pDocMgrFocus != nullptr)) {
        _InitTextEditSink(pDocMgrFocus);
        pDocMgrFocus->Release();
    }

    if (!_InitKeyEventSink()) {
        goto ExitError;
    }

    if (!_InitThreadFocusSink()) {
        goto ExitError;
    }

    if (!_InitFunctionProviderSink()) {
        goto ExitError;
    }

    return S_OK;

ExitError:
    Deactivate();
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx::Deactivate
//
//----------------------------------------------------------------------------

STDAPI IMECore::Deactivate() {
    _UninitFunctionProviderSink();

    _UninitThreadFocusSink();

    _UninitKeyEventSink();

    _UninitThreadMgrEventSink();

    if (_pThreadMgr != nullptr) {
        _pThreadMgr->Release();
    }

    _tfClientId = TF_CLIENTID_NULL;

    if (_pDocMgrLastFocused) {
        _pDocMgrLastFocused->Release();
        _pDocMgrLastFocused = nullptr;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::GetType
//
//----------------------------------------------------------------------------
HRESULT IMECore::GetType(__RPC__out GUID *pguid) {
    HRESULT hr = E_INVALIDARG;
    if (pguid) {
        *pguid = Global::IMECLSID;
        hr = S_OK;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::::GetDescription
//
//----------------------------------------------------------------------------
HRESULT IMECore::GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc) {
    HRESULT hr = E_INVALIDARG;
    if (pbstrDesc != nullptr) {
        *pbstrDesc = nullptr;
        hr = E_NOTIMPL;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::::GetFunction
//
//----------------------------------------------------------------------------
HRESULT IMECore::GetFunction(
    __RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk) {
    HRESULT hr = E_NOINTERFACE;

    if (IsEqualGUID(rguid, GUID_NULL)) {
        hr = QueryInterface(riid, (void **)ppunk);
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunction::GetDisplayName
//
//----------------------------------------------------------------------------
HRESULT IMECore::GetDisplayName(_Out_ BSTR *pbstrDisplayName) {
    HRESULT hr = E_INVALIDARG;
    if (pbstrDisplayName != nullptr) {
        *pbstrDisplayName = nullptr;
        hr = E_NOTIMPL;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFnGetPreferredTouchKeyboardLayout::GetLayout
// The tkblayout will be Optimized layout.
//----------------------------------------------------------------------------
HRESULT
IMECore::GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId) {
    return E_NOTIMPL;
}
