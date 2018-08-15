// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#include "VirtualDocument.h"

namespace VietType {
namespace VirtualDocument {

_Check_return_ HRESULT GetVirtualDocumentMgr(_In_ ITfDocumentMgr* dim, _Outptr_ ITfDocumentMgr** pdim) {
    HRESULT hr;

    CComPtr<ITfContext> context;
    hr = dim->GetTop(&context);
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
        CComPtr<ITfCompartmentMgr> tcMgr;
        hr = dim->QueryInterface(&tcMgr);
        HRESULT_CHECK_RETURN(hr, L"%s", L"dim->QueryInterface failed");

        CComPtr<ITfCompartment> transitoryCompartment;
        hr = tcMgr->GetCompartment(GUID_COMPARTMENT_TRANSITORYEXTENSION_PARENT, &transitoryCompartment);
        HRESULT_CHECK_RETURN(hr, L"%s", L"tcMgr->GetCompartment failed");

        VARIANT v;
        hr = transitoryCompartment->GetValue(&v);
        HRESULT_CHECK_RETURN(hr, L"%s", L"transitoryCompartment->GetValue failed");
        if (v.vt != VT_UNKNOWN || !v.punkVal) {
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

_Check_return_ HRESULT GetVirtualDocumentContext(_In_ ITfContext* context, _Outptr_ ITfContext** pContext) {
    HRESULT hr;

    CComPtr<ITfDocumentMgr> dim;
    hr = context->GetDocumentMgr(&dim);
    HRESULT_CHECK_RETURN(hr, L"%s", L"GetTransitoryParentDocumentMgr failed");

    if (!dim) {
        return E_NOINTERFACE;
    }

    CComPtr<ITfDocumentMgr> pdim;
    hr = GetVirtualDocumentMgr(dim, &pdim);
    HRESULT_CHECK_RETURN(hr, L"%s", L"GetTransitoryParentDocumentMgr failed");

    return pdim->GetTop(pContext);
}

}
}
