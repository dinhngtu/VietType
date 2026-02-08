// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "VirtualDocument.h"
#include "Compartment.h"

namespace VietType {
namespace VirtualDocument {

static const GUID GUID_Compartment_TsfEmulatedDocumentMgr = {
    0xa94c5fd2, 0xc471, 0x4031, {0x95, 0x46, 0x70, 0x9c, 0x17, 0x30, 0x0c, 0xb9}};

static _Check_return_ HRESULT
GetVirtualDocumentMgr(_In_ ITfDocumentMgr* dim, _Outptr_result_maybenull_ ITfDocumentMgr** virtualDim) {
    HRESULT hr;

    CComPtr<ITfCompartmentMgr> tcMgr;
    hr = dim->QueryInterface(&tcMgr);
    HRESULT_CHECK_RETURN_OUTPTR(hr, virtualDim, L"dim->QueryInterface failed");

    CComPtr<ITfCompartment> transitoryCompartment;
    hr = tcMgr->GetCompartment(GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT, &transitoryCompartment);
    // no parent, might be Chromium?
    if (FAILED(hr)) {
        *virtualDim = nullptr;
        return S_FALSE;
    }

    CComVariant v;
    hr = transitoryCompartment->GetValue(&v);
    HRESULT_CHECK_RETURN_OUTPTR(hr, virtualDim, L"transitoryCompartment->GetValue failed");
    if (v.vt != VT_UNKNOWN || !v.punkVal) {
        *virtualDim = nullptr;
        return E_NOINTERFACE;
    }

    hr = v.punkVal->QueryInterface(virtualDim);
    HRESULT_CHECK_RETURN_OUTPTR(hr, virtualDim, L"punkVal->QueryInterface failed");
    return S_OK;
}

_Check_return_ HRESULT GetVirtualDocumentContext(
    _In_ ITfContext* context, _In_ ITfDocumentMgr* dim, _Outptr_result_maybenull_ ITfContext** pContext) {
    HRESULT hr;

    CComPtr<ITfDocumentMgr> virtualDim;
    hr = GetVirtualDocumentMgr(dim, &virtualDim);
    if (hr == S_FALSE) {
        *pContext = nullptr;
        return hr;
    }
    HRESULT_CHECK_RETURN_OUTPTR(hr, pContext, L"GetVirtualDocumentMgr failed");

    hr = virtualDim->GetTop(pContext);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pContext, L"pdim->GetTop failed");

    return S_OK;
}

_Check_return_ HRESULT GetFullContext(
    _In_ ITfContext* context,
    _In_ TfClientId clientId,
    _COM_Outptr_ ITfContext** fullContext,
    _Out_ FullContextType* contextType) {
    HRESULT hr;

    CComPtr<ITfDocumentMgr> dim;
    hr = context->GetDocumentMgr(&dim);
    HRESULT_CHECK_RETURN_OUTPTR(hr, fullContext, L"context->GetDocumentMgr failed");
    if (!dim) {
        *fullContext = nullptr;
        return E_NOINTERFACE;
    }

    TF_STATUS st;
    hr = context->GetStatus(&st);
    HRESULT_CHECK_RETURN_OUTPTR(hr, fullContext, L"context->GetStatus failed");
    if (!(st.dwStaticFlags & TF_SS_TRANSITORY)) {
        *fullContext = context;
        (*fullContext)->AddRef();
        *contextType = FullContextType::Original;
        return S_OK;
    }

    hr = VirtualDocument::GetVirtualDocumentContext(context, dim, fullContext);
    HRESULT_CHECK_RETURN_OUTPTR(hr, fullContext, L"VirtualDocument::GetVirtualDocumentContext failed");
    if (hr == S_FALSE) {
        // GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT not found, *fullContext should be nullptr
        Compartment<long> emulated;
        long emulatedVal;
        hr = emulated.Initialize(dim, clientId, GUID_Compartment_TsfEmulatedDocumentMgr);
        if (SUCCEEDED(hr) && SUCCEEDED(emulated.GetValue(&emulatedVal)) && (emulatedVal & 1)) {
            // emulated
            return E_NOTIMPL;
        } else {
            *fullContext = context;
            (*fullContext)->AddRef();
            *contextType = FullContextType::Chromium;
            return S_OK;
        }
    } else {
        // succeeded
        hr = (*fullContext)->GetStatus(&st);
        HRESULT_CHECK_RETURN_OUTPTR(hr, fullContext, L"fullContext->GetStatus failed");
        if (st.dwStaticFlags & TF_SS_TRANSITORY) {
            (*fullContext)->Release();
            *fullContext = nullptr;
            return E_FAIL;
        } else {
            *contextType = FullContextType::Transitory;
            return S_OK;
        }
    }
}

} // namespace VirtualDocument
} // namespace VietType
