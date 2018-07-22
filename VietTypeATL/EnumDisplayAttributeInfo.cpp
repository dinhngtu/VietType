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

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "EnumDisplayAttributeInfo.h"

VietType::EnumDisplayAttributeInfo::EnumDisplayAttributeInfo() {
}

VietType::EnumDisplayAttributeInfo::~EnumDisplayAttributeInfo() {
}

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Clone(IEnumTfDisplayAttributeInfo ** ppEnum) {
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

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Next(ULONG ulCount, ITfDisplayAttributeInfo ** rgInfo, ULONG * pcFetched) {
    auto index_old = _index;
    for (ULONG i = 0; i < ulCount; i++) {
        if (_index >= _items.size()) {
            break;
        }
        *rgInfo = _items.at(_index);
        (*rgInfo)->AddRef();
        rgInfo++;
        _index++;
    }
    auto fetched = _index - index_old;
    if (pcFetched) {
        *pcFetched = static_cast<ULONG>(fetched);
    }
    return (fetched == ulCount) ? S_OK : S_FALSE;
}

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Reset(void) {
    _index = 0;
    return S_OK;
}

STDMETHODIMP VietType::EnumDisplayAttributeInfo::Skip(ULONG ulCount) {
    _index += ulCount;
    return (_index < _items.size()) ? S_OK : S_FALSE;
}

void VietType::EnumDisplayAttributeInfo::Initialize(info_vector_type const & items, info_vector_type::size_type index) {
    _items = items;
    _index = index;
}

void VietType::EnumDisplayAttributeInfo::AddAttribute(ITfDisplayAttributeInfo* item) {
    _items.push_back(item);
}

ITfDisplayAttributeInfo* VietType::EnumDisplayAttributeInfo::GetAttribute(info_vector_type::size_type index) {
    return _items.at(index);
}

HRESULT VietType::EnumDisplayAttributeInfo::FindAttributeByGuid(GUID const & guid, ITfDisplayAttributeInfo ** info) {
    HRESULT hr;
    *info = nullptr;

    for (auto const& attr : _items) {
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
