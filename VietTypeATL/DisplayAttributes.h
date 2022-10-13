// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class DisplayAttributeInfo : public CComObjectRootEx<CComSingleThreadModel>, public ITfDisplayAttributeInfo {
public:
    DisplayAttributeInfo() = default;
    DisplayAttributeInfo(const DisplayAttributeInfo&) = delete;
    DisplayAttributeInfo& operator=(const DisplayAttributeInfo&) = delete;
    ~DisplayAttributeInfo() = default;

    DECLARE_NOT_AGGREGATABLE(DisplayAttributeInfo)
    BEGIN_COM_MAP(DisplayAttributeInfo)
    COM_INTERFACE_ENTRY(ITfDisplayAttributeInfo)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    // Inherited via ITfDisplayAttributeInfo
    virtual STDMETHODIMP GetGUID(__RPC__out GUID* pguid) override;
    virtual STDMETHODIMP GetDescription(__RPC__deref_out_opt BSTR* pbstrDesc) override;
    virtual STDMETHODIMP GetAttributeInfo(__RPC__out TF_DISPLAYATTRIBUTE* pda) override;
    virtual STDMETHODIMP SetAttributeInfo(__RPC__in const TF_DISPLAYATTRIBUTE* pda) override;
    virtual STDMETHODIMP Reset(void) override;

public:
    HRESULT Initialize(_In_ const GUID& guid, _In_ std::wstring description, _In_ TF_DISPLAYATTRIBUTE attr);

private:
    GUID _guid = {0};
    std::wstring _description;
    TF_DISPLAYATTRIBUTE _attr = TF_DISPLAYATTRIBUTE();
    TF_DISPLAYATTRIBUTE _attrOrig = TF_DISPLAYATTRIBUTE();
};

} // namespace VietType
