#include "VirtualDocument.h"

HRESULT VietType::VirtualDocument::GetVirtualDocumentMgr(ITfDocumentMgr * dim, ITfDocumentMgr ** pdim) {
    HRESULT hr;

    SmartComPtr<ITfContext> context;
    hr = dim->GetTop(context.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"pdimFocus->GetTop failed");

    if (!context) {
        *pdim = dim;
        (*pdim)->AddRef();
        return S_OK;
    }

    TF_STATUS st;
    hr = context->GetStatus(&st);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetStatus failed");

    if (st.dwStaticFlags & TF_SS_TRANSITORY) {
        SmartComPtr<ITfCompartmentMgr> tcMgr(dim);
        if (!tcMgr) {
            return E_NOINTERFACE;
        }

        SmartComPtr<ITfCompartment> transitoryCompartment;
        hr = tcMgr->GetCompartment(GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT, transitoryCompartment.GetAddress());
        HRESULT_CHECK_RETURN(hr, L"%s", L"tcMgr->GetCompartment failed");

        VARIANT v;
        hr = transitoryCompartment->GetValue(&v);
        HRESULT_CHECK_RETURN(hr, L"%s", L"transitoryCompartment->GetValue failed");
        if (v.vt != VT_UNKNOWN || !v.punkVal) {
            DBG_DPRINT(L"bad variant type %d", static_cast<int>(v.vt));
            return E_NOINTERFACE;
        }

        hr = v.punkVal->QueryInterface<ITfDocumentMgr>(pdim);
        VariantClear(&v);
        return hr;
    } else {
        *pdim = dim;
        (*pdim)->AddRef();
        return S_OK;
    }
}

HRESULT VietType::VirtualDocument::GetVirtualDocumentContext(ITfContext * context, ITfContext ** pContext) {
    HRESULT hr;

    SmartComPtr<ITfDocumentMgr> dim;
    hr = context->GetDocumentMgr(dim.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"GetTransitoryParentDocumentMgr failed");

    if (!dim) {
        return E_NOINTERFACE;
    }

    SmartComPtr<ITfDocumentMgr> pdim;
    hr = GetVirtualDocumentMgr(dim, pdim.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"GetTransitoryParentDocumentMgr failed");

    return pdim->GetTop(pContext);
}
