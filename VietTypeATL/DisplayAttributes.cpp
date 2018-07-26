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

#include "DisplayAttributes.h"

const std::tuple<GUID, std::wstring, TF_DISPLAYATTRIBUTE> VietType::ComposingAttributeData = std::make_tuple(
    // {7AB7384D-F5C6-43F9-B13C-80DCC788EE1D}
    GUID{ 0x7ab7384d, 0xf5c6, 0x43f9, { 0xb1, 0x3c, 0x80, 0xdc, 0xc7, 0x88, 0xee, 0x1d } },
    std::wstring(L"Composing"),
    TF_DISPLAYATTRIBUTE{
        { TF_CT_NONE, 0 }, // text foreground
        { TF_CT_NONE, 0 }, // text background
        TF_LS_DOT, // underline style
        FALSE, // bold underline
        { TF_CT_NONE, 0 }, // underline color
        TF_ATTR_INPUT // attribute info
    }
);

#pragma warning(push)
#pragma warning(disable: 26495)
VietType::DisplayAttributeInfo::DisplayAttributeInfo() noexcept {
#pragma warning(pop)
}

VietType::DisplayAttributeInfo::~DisplayAttributeInfo() {
}

STDMETHODIMP VietType::DisplayAttributeInfo::GetGUID(__RPC__out GUID* pguid) {
    *pguid = _guid;
    return S_OK;
}

STDMETHODIMP VietType::DisplayAttributeInfo::GetDescription(__RPC__deref_out_opt BSTR* pbstrDesc) {
    *pbstrDesc = SysAllocString(_description.c_str());
    return *pbstrDesc ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP VietType::DisplayAttributeInfo::GetAttributeInfo(__RPC__out TF_DISPLAYATTRIBUTE* pda) {
    *pda = _attr;
    return S_OK;
}

STDMETHODIMP VietType::DisplayAttributeInfo::SetAttributeInfo(__RPC__in const TF_DISPLAYATTRIBUTE* pda) {
    _attr = *pda;
    return S_OK;
}

STDMETHODIMP VietType::DisplayAttributeInfo::Reset(void) {
    _attr = _attrOrig;
    return S_OK;
}

HRESULT VietType::DisplayAttributeInfo::Initialize(_In_ const GUID& guid, _In_ std::wstring description, _In_ TF_DISPLAYATTRIBUTE attr) {
    _guid = guid;
    _description = description;
    _attr = attr;
    _attrOrig = attr;

    return S_OK;
}
