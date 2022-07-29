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

#include "Compartment.h"

namespace VietType {

_Ret_maybenull_ ITfCompartment* CompartmentBase::GetCompartment() {
    return _compartment;
}

_Check_return_ HRESULT CompartmentBase::GetCompartmentSource(_COM_Outptr_ ITfSource** ppSource) {
    return _compartment->QueryInterface(ppSource);
}

_Check_return_ HRESULT CompartmentBase::Initialize(
    _In_ IUnknown* punk, _In_ TfClientId clientid, _In_ const GUID& guidCompartment, _In_ bool global) {
    HRESULT hr;

    CComPtr<ITfCompartmentMgr> compartmentMgr;

    if (global) {
        CComPtr<ITfThreadMgr> threadMgr;
        hr = punk->QueryInterface(&threadMgr);
        HRESULT_CHECK_RETURN(hr, L"%s", L"punk->QueryInterface failed");

        hr = threadMgr->GetGlobalCompartment(&compartmentMgr);
        HRESULT_CHECK_RETURN(hr, L"%s", L"threadMgr->GetGlobalCompartment failed");
    } else {
        hr = punk->QueryInterface(&compartmentMgr);
        HRESULT_CHECK_RETURN(hr, L"%s", L"punk->QueryInterface failed");
    }

    hr = compartmentMgr->GetCompartment(guidCompartment, &_compartment);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compartmentMgr->GetCompartment failed");

#ifdef _DEBUG
    if (global) {
        HRESULT dbgHr;
        VARIANT v;
        dbgHr = _compartment->GetValue(&v);
        DBG_HRESULT_CHECK(dbgHr, L"%s", L"test _compartment->GetValue failed");
        DBG_DPRINT(L"created global compartment vartype=%u long=%ld", v.vt, v.lVal);
        CComPtr<IEnumGUID> globalCompartments;
        dbgHr = compartmentMgr->EnumCompartments(&globalCompartments);
        DBG_HRESULT_CHECK(dbgHr, L"%s", L"compartmentMgr->EnumCompartments failed");
        while (1) {
            GUID guid;
            dbgHr = globalCompartments->Next(1, &guid, NULL);
            if (dbgHr == S_OK) {
                DBG_DPRINT(L"global compartment " GUID_WFORMAT, GUID_COMPONENTS(guid));
            } else {
                break;
            }
        }
    }
#endif // _DEBUG

    _clientid = clientid;

    return S_OK;
}

HRESULT CompartmentBase::Uninitialize() {
    _compartment.Release();

    return S_OK;
}

} // namespace VietType
