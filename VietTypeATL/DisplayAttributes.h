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

extern const std::tuple<GUID, std::wstring, TF_DISPLAYATTRIBUTE> ComposingAttributeData;

class DisplayAttributeInfo :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfDisplayAttributeInfo {
public:
    DisplayAttributeInfo() noexcept;
    ~DisplayAttributeInfo();

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
    void Initialize(_In_ const GUID& guid, _In_ std::wstring description, _In_ TF_DISPLAYATTRIBUTE attr);

private:
    GUID _guid = { 0 };
    std::wstring _description;
    TF_DISPLAYATTRIBUTE _attr;
    TF_DISPLAYATTRIBUTE _attrOrig;

private:
    DISALLOW_COPY_AND_ASSIGN(DisplayAttributeInfo);
};

}
