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

// Derived from Microsoft's SampleIME source code included in the Windows classic samples:
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "EnumDisplayAttributeInfo.h"

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Clone(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) {
    HRESULT hr;
    *ppEnum = nullptr;

    CComPtr<EnumDisplayAttributeInfo> ret;
    hr = CreateInstance2(&ret);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&ret) failed");
    ret->Initialize(_items, _index);

    *ppEnum = ret;
    (*ppEnum)->AddRef();
    return S_OK;
}

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Next(_In_ ULONG ulCount, __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo** rgInfo, __RPC__out ULONG* pcFetched) {
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

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Reset(void) {
    _index = 0;
    return S_OK;
}

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Skip(_In_ ULONG ulCount) {
    _index += ulCount;
    return (_index < _items.size()) ? S_OK : S_FALSE;
}

void VietType::EnumDisplayAttributeInfo::Initialize(_In_ const info_vector_type& items, _In_ info_vector_type::size_type index) {
    _items = items;
    _index = index;
}

void VietType::EnumDisplayAttributeInfo::AddAttribute(_In_ ITfDisplayAttributeInfo* item) {
    _items.push_back(CComPtr<ITfDisplayAttributeInfo>(item));
}

_Ret_valid_ ITfDisplayAttributeInfo* VietType::EnumDisplayAttributeInfo::GetAttribute(_In_ info_vector_type::size_type index) {
    return _items.at(index);
}

_Check_return_ HRESULT VietType::EnumDisplayAttributeInfo::FindAttributeByGuid(_In_ const GUID& guid, _Outptr_ ITfDisplayAttributeInfo** info) {
    HRESULT hr;
    *info = nullptr;

    for (const auto& attr : _items) {
        GUID aguid;
        hr = attr->GetGUID(&aguid);
        HRESULT_CHECK_RETURN(hr, L"%s", L"attr->GetGUID");
        if (IsEqualGUID(guid, aguid)) {
            *info = attr;
            (*info)->AddRef();
            return S_OK;
        }
    }

    // the docs for GetDisplayAttributeInfo doesn't say which error to use when no attribute is found so we just use this one
    return E_INVALIDARG;
}
