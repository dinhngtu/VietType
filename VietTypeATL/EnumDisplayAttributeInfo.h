// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class EnumDisplayAttributeInfo : public CComObjectRootEx<CComSingleThreadModel>, public IEnumTfDisplayAttributeInfo {
public:
    using info_vector_type = std::vector<CComPtr<ITfDisplayAttributeInfo>>;

    EnumDisplayAttributeInfo() = default;
    EnumDisplayAttributeInfo(const EnumDisplayAttributeInfo&) = delete;
    EnumDisplayAttributeInfo& operator=(const EnumDisplayAttributeInfo&) = delete;
    ~EnumDisplayAttributeInfo() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(EnumDisplayAttributeInfo)
    BEGIN_COM_MAP(EnumDisplayAttributeInfo)
    COM_INTERFACE_ENTRY(IEnumTfDisplayAttributeInfo)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    virtual STDMETHODIMP Clone(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) override;
    virtual STDMETHODIMP Next(
        _In_ ULONG ulCount,
        __RPC__out_ecount_part(ulCount, *pcFetched) ITfDisplayAttributeInfo** rgInfo,
        __RPC__out ULONG* pcFetched) override;
    virtual STDMETHODIMP Reset(void) override;
    virtual STDMETHODIMP Skip(_In_ ULONG ulCount) override;

public:
    // only call when copying from another EnumDisplayAttributeInfo
    HRESULT Initialize(_In_ const info_vector_type& items, _In_ info_vector_type::size_type index);
    // dummy method
    HRESULT Uninitialize();

    void AddAttribute(_In_ ITfDisplayAttributeInfo* item);
    _Ret_valid_ ITfDisplayAttributeInfo* GetAttribute(_In_ info_vector_type::size_type index);
    _Check_return_ HRESULT FindAttributeByGuid(_In_ const GUID& guid, _COM_Outptr_ ITfDisplayAttributeInfo** info);

private:
    info_vector_type _items;
    info_vector_type::size_type _index = 0;
};

} // namespace VietType
