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

#pragma once

#include "Common.h"

namespace VietType {

class CompartmentBase {
public:
    CompartmentBase() = default;
    CompartmentBase(const CompartmentBase&) = delete;
    CompartmentBase& operator=(const CompartmentBase&) = delete;
    ~CompartmentBase() = default;

public:
    _Ret_valid_ ITfCompartment* GetCompartment();
    _Check_return_ HRESULT GetCompartmentSource(_COM_Outptr_ ITfSource** ppSource);

    _Check_return_ HRESULT Initialize(_In_ IUnknown* punk, _In_ TfClientId clientid, _In_ const GUID& guidCompartment, _In_ bool global = false);
    HRESULT Uninitialize();

protected:
    CComPtr<ITfCompartment> _compartment;
    TfClientId _clientid = TF_CLIENTID_NULL;

    template <typename T>
    struct variantInfo {
        static constexpr VARTYPE vartype = VT_ILLEGAL;
        static constexpr T& accessor(VARIANT& v) = delete;
    };
};

template <typename T>
class Compartment : public CompartmentBase {
public:
    _Check_return_ _Success_(return == S_OK) HRESULT GetValue(_Out_ T* val) {
        HRESULT hr;

        VARIANT v;
        hr = _compartment->GetValue(&v);
        if (hr == S_FALSE) {
            return S_FALSE;
        } else if (hr == S_OK) {
            if (v.vt != variantInfo<T>::vartype) {
                return E_FAIL;
            }
            *val = variantInfo<T>::accessor(v);
        }

        return hr;
    }

    HRESULT SetValue(const T& val) {
        HRESULT hr;

        VARIANT v;
        VariantInit(&v);
        v.vt = variantInfo<T>::vartype;
        variantInfo<T>::accessor(v) = val;
        hr = _compartment->SetValue(_clientid, &v);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_compartment->SetValue failed");

        return S_OK;
    }

    _Check_return_ HRESULT GetValueOrWriteback(_Out_ T* val, const T& defaultValue) {
        HRESULT hr = GetValue(val);
        if (hr == S_FALSE) {
            hr = SetValue(defaultValue);
            if (SUCCEEDED(hr)) {
                *val = defaultValue;
                return S_FALSE;
            }
            return hr;
        }
        return hr;
    }
};

template <>
struct CompartmentBase::variantInfo<long> {
    static constexpr VARTYPE vartype = VT_I4;
    static constexpr long& accessor(VARIANT& v) {
        return v.lVal;
    }
};

}
