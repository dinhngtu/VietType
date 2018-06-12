// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "IMECore.h"
#include "Define.h"

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

static Telex::TelexConfig TELEX_DEFAULT_CONFIG = {
    true,
};

IMECore::IMECore() noexcept : _engine(TELEX_DEFAULT_CONFIG) {
    DllAddRef();

    _pThreadMgr = nullptr;
    _tfClientId = TF_CLIENTID_NULL;
    _dwActivateFlags = 0;

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;

    _pTextEditSinkContext = nullptr;
    _textEditSinkCookie = TF_INVALID_COOKIE;

    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

    _pDocMgrLastFocused = nullptr;

    _pContext = nullptr;

    _pComposition = nullptr;

    _disabled = 0;

    _refCount = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

IMECore::~IMECore() {
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
// ITfFunction::GetDisplayName
//
//----------------------------------------------------------------------------
HRESULT IMECore::GetDisplayName(_Out_ BSTR *pbstrDisplayName) {
    if (!pbstrDisplayName) {
        return E_INVALIDARG;
    }
    BSTR ret = SysAllocString(TEXTSERVICE_DISPLAYNAME);
    if (ret) {
        *pbstrDisplayName = ret;
        return S_OK;
    } else {
        return E_OUTOFMEMORY;
    }
}
