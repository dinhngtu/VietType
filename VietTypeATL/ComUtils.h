// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <typeinfo>
#include <atlbase.h>

template <typename T>
_Check_return_ HRESULT CreateInstance2(_COM_Outptr_ T** ppout) {
    ATL::CComObject<T>* p;
    HRESULT hr = ATL::CComObject<T>::CreateInstance(&p);
    if (SUCCEEDED(hr)) {
        p->AddRef();
        *ppout = p;
    } else {
        *ppout = nullptr;
    }
    return hr;
}

template <typename T, typename... Args>
_Check_return_ HRESULT CreateInitialize(_COM_Outptr_ T** ppout, Args... args) {
    ATL::CComObject<T>* p;
    HRESULT hr = ATL::CComObject<T>::CreateInstance(&p);
    if (SUCCEEDED(hr)) {
        p->AddRef();
        *ppout = p;
        hr = (*ppout)->Initialize(args...);
        if (FAILED(hr)) {
            HRESULT_CHECK(hr, L"%hs->Initialize failed", typeid(T).name());
            p->Release();
            *ppout = nullptr;
        }
    } else {
        *ppout = nullptr;
    }
    return hr;
}

template <typename TFrom, typename TTo>
_Check_return_ HRESULT QueryInterface2(_In_ TFrom* from, _COM_Outptr_ TTo** to) {
    return from->QueryInterface(__uuidof(TTo), reinterpret_cast<void**>(to));
}
