// SPDX-License-Identifier: GPL-3.0-only

#include "VirtualDocument.h"

namespace VietType {
namespace VirtualDocument {

static _Check_return_ HRESULT GetVirtualDocumentMgr(_In_ ITfDocumentMgr* dim, _COM_Outptr_ ITfDocumentMgr** pdim) {
    HRESULT hr;

    CComPtr<ITfCompartmentMgr> tcMgr;
    hr = dim->QueryInterface(&tcMgr);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"dim->QueryInterface failed");

    CComPtr<ITfCompartment> transitoryCompartment;
    hr = tcMgr->GetCompartment(GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT, &transitoryCompartment);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"tcMgr->GetCompartment failed");

    CComVariant v;
    hr = transitoryCompartment->GetValue(&v);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"transitoryCompartment->GetValue failed");
    if (v.vt != VT_UNKNOWN || !v.punkVal) {
        *pdim = nullptr;
        return E_NOINTERFACE;
    }

    hr = v.punkVal->QueryInterface(pdim);
    return hr;
}

_Check_return_ HRESULT GetVirtualDocumentContext(_In_ ITfContext* context, _COM_Outptr_ ITfContext** pContext) {
    HRESULT hr;

    TF_STATUS st;
    hr = context->GetStatus(&st);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pContext, L"%s", L"context->GetStatus failed");

    if (!(st.dwStaticFlags & TF_SS_TRANSITORY)) {
        *pContext = nullptr;
        return E_NOINTERFACE;
    }

    CComPtr<ITfDocumentMgr> dim;
    hr = context->GetDocumentMgr(&dim);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pContext, L"%s", L"GetTransitoryParentDocumentMgr failed");

    if (!dim) {
        *pContext = nullptr;
        return E_NOINTERFACE;
    }

    CComPtr<ITfDocumentMgr> pdim;
    hr = GetVirtualDocumentMgr(dim, &pdim);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pContext, L"%s", L"GetTransitoryParentDocumentMgr failed");

    hr = pdim->GetTop(pContext);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pContext, L"%s", L"pdim->GetTop failed");

    return S_OK;
}

} // namespace VirtualDocument
} // namespace VietType
