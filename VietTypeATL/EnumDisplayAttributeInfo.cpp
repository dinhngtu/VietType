// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
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

#include "EnumDisplayAttributeInfo.h"

namespace VietType {

STDMETHODIMP EnumDisplayAttributeInfo::Clone(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) {
    HRESULT hr;
    *ppEnum = nullptr;

    CComPtr<EnumDisplayAttributeInfo> ret;
    hr = CreateInitialize(&ret, _items, _index);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&ret) failed");

    *ppEnum = ret;
    (*ppEnum)->AddRef();
    return S_OK;
}

STDMETHODIMP EnumDisplayAttributeInfo::Next(
    _In_ ULONG ulCount,
    __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo** rgInfo,
    __RPC__out ULONG* pcFetched) {
    ULONG i;
    for (i = 0; i < ulCount; i++) {
        if (_index >= _items.size()) {
            break;
        }
        rgInfo[i] = _items.at(_index);
        (*rgInfo)->AddRef();
        _index++;
    }
    if (pcFetched) {
        *pcFetched = i;
    }
    return (i == ulCount) ? S_OK : S_FALSE;
}

STDMETHODIMP EnumDisplayAttributeInfo::Reset(void) {
    _index = 0;
    return S_OK;
}

STDMETHODIMP EnumDisplayAttributeInfo::Skip(_In_ ULONG ulCount) {
    _index += ulCount;
    return (_index < _items.size()) ? S_OK : S_FALSE;
}

HRESULT EnumDisplayAttributeInfo::Initialize(
    _In_ const info_vector_type& items, _In_ info_vector_type::size_type index) {
    _items = items;
    _index = index;
    return S_OK;
}

HRESULT EnumDisplayAttributeInfo::Uninitialize() {
    return S_OK;
}

void EnumDisplayAttributeInfo::AddAttribute(_In_ ITfDisplayAttributeInfo* item) {
    _items.push_back(CComPtr<ITfDisplayAttributeInfo>(item));
}

_Ret_valid_ ITfDisplayAttributeInfo* EnumDisplayAttributeInfo::GetAttribute(_In_ info_vector_type::size_type index) {
    return _items.at(index);
}

_Check_return_ HRESULT
EnumDisplayAttributeInfo::FindAttributeByGuid(_In_ const GUID& guid, _COM_Outptr_ ITfDisplayAttributeInfo** info) {
    HRESULT hr;
    *info = nullptr;

    for (const auto& attr : _items) {
        GUID aguid;
        hr = attr->GetGUID(&aguid);
        HRESULT_CHECK_RETURN(hr, L"%s", L"attr->GetGUID");
        if (guid == aguid) {
            *info = attr;
            (*info)->AddRef();
            return S_OK;
        }
    }

    // the docs for GetDisplayAttributeInfo doesn't say which error to use when no attribute is found so we just use
    // this one
    return E_INVALIDARG;
}

} // namespace VietType
