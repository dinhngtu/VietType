// SPDX-License-Identifier: GPL-3.0-only

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
    _Ret_maybenull_ ITfCompartment* GetCompartment();
    _Check_return_ HRESULT GetCompartmentSource(_COM_Outptr_ ITfSource** ppSource);

    _Check_return_ HRESULT Initialize(
        _In_ IUnknown* punk, _In_ TfClientId clientid, _In_ const GUID& guidCompartment, _In_ bool global = false);
    HRESULT Uninitialize();

protected:
    CComPtr<ITfCompartment> _compartment;
    TfClientId _clientid = TF_CLIENTID_NULL;

    template <typename T>
    struct variantInfo {
        static constexpr VARTYPE vartype = VT_ILLEGAL;
        static constexpr T& accessor(CComVariant& v) = delete;
    };
};

// ITfCompartment only supports VT_I4, VT_UNKNOWN and VT_BSTR
template <typename T>
class Compartment : public CompartmentBase {
public:
    _Check_return_ _Success_(return == S_OK) HRESULT GetValue(_Out_ T* val) {
        HRESULT hr;

        CComVariant v;
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

        CComVariant v;
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
    static constexpr long& accessor(CComVariant& v) {
        return v.lVal;
    }
};

template <>
struct CompartmentBase::variantInfo<IUnknown*> {
    static constexpr VARTYPE vartype = VT_UNKNOWN;
    static constexpr IUnknown*& accessor(CComVariant& v) {
        return v.punkVal;
    }
};

} // namespace VietType
