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

class EnumDisplayAttributeInfo :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IEnumTfDisplayAttributeInfo {
public:
    using info_vector_type = std::vector<CComPtr<ITfDisplayAttributeInfo>>;

    EnumDisplayAttributeInfo();
    ~EnumDisplayAttributeInfo();

    DECLARE_NOT_AGGREGATABLE(EnumDisplayAttributeInfo)
    BEGIN_COM_MAP(EnumDisplayAttributeInfo)
        COM_INTERFACE_ENTRY(IEnumTfDisplayAttributeInfo)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    virtual STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo** ppEnum) override;
    virtual STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo** rgInfo, ULONG* pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(ULONG ulCount) override;

public:
    void Initialize(const info_vector_type& items, info_vector_type::size_type index);

    void AddAttribute(ITfDisplayAttributeInfo* item);
    ITfDisplayAttributeInfo* GetAttribute(info_vector_type::size_type index);
    HRESULT FindAttributeByGuid(const GUID& guid, ITfDisplayAttributeInfo** info);

private:
    info_vector_type _items;
    info_vector_type::size_type _index = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(EnumDisplayAttributeInfo);
};

}
