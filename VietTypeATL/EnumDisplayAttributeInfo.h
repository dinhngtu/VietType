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

    EnumDisplayAttributeInfo() noexcept;
    ~EnumDisplayAttributeInfo();

    DECLARE_NOT_AGGREGATABLE(EnumDisplayAttributeInfo)
    BEGIN_COM_MAP(EnumDisplayAttributeInfo)
        COM_INTERFACE_ENTRY(IEnumTfDisplayAttributeInfo)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    virtual STDMETHODIMP Clone(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) override;
    virtual STDMETHODIMP Next(_In_ ULONG ulCount, __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo** rgInfo, __RPC__out ULONG* pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(_In_ ULONG ulCount) override;

public:
    HRESULT Initialize(_In_ const info_vector_type& items, _In_ info_vector_type::size_type index);

    void AddAttribute(_In_ ITfDisplayAttributeInfo* item);
    _Ret_valid_ ITfDisplayAttributeInfo* GetAttribute(_In_ info_vector_type::size_type index);
    _Check_return_ HRESULT FindAttributeByGuid(_In_ const GUID& guid, _Outptr_ ITfDisplayAttributeInfo** info);

private:
    info_vector_type _items;
    info_vector_type::size_type _index = 0;

private:
    DISALLOW_COPY_AND_ASSIGN(EnumDisplayAttributeInfo);
};

}
