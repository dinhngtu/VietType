// SPDX-License-Identifier: GPL-3.0-only

#include "VirtualDocument.h"

namespace VietType {
namespace VirtualDocument {

_Check_return_ HRESULT GetVirtualDocumentMgr(_In_ ITfDocumentMgr* dim, _COM_Outptr_ ITfDocumentMgr** pdim) {
    HRESULT hr;

    CComPtr<ITfContext> context;
    hr = dim->GetTop(&context);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"pdimFocus->GetTop failed");

    if (!context) {
        *pdim = dim;
        (*pdim)->AddRef();
        return S_OK;
    }

    TF_STATUS st;
    hr = context->GetStatus(&st);
    HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"context->GetStatus failed");

    if (st.dwStaticFlags & TF_SS_TRANSITORY) {
        CComPtr<ITfCompartmentMgr> tcMgr;
        hr = dim->QueryInterface(&tcMgr);
        HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"dim->QueryInterface failed");

        CComPtr<ITfCompartment> transitoryCompartment;
        hr = tcMgr->GetCompartment(GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT, &transitoryCompartment);
        HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"tcMgr->GetCompartment failed");

        VARIANT v;
        hr = transitoryCompartment->GetValue(&v);
        HRESULT_CHECK_RETURN_OUTPTR(hr, pdim, L"%s", L"transitoryCompartment->GetValue failed");
        if (v.vt != VT_UNKNOWN || !v.punkVal) {
            *pdim = nullptr;
            DBG_DPRINT(L"bad variant type %d", static_cast<int>(v.vt));
            return E_NOINTERFACE;
        }

        hr = v.punkVal->QueryInterface(pdim);
        VariantClear(&v);
        return hr;
    } else {
        *pdim = dim;
        (*pdim)->AddRef();
        return S_OK;
    }
}

_Check_return_ HRESULT GetVirtualDocumentContext(_In_ ITfContext* context, _COM_Outptr_ ITfContext** pContext) {
    HRESULT hr;

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
