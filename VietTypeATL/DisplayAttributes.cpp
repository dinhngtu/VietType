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

std::tuple<GUID, std::wstring, TF_DISPLAYATTRIBUTE> const VietType::ComposingAttributeData = std::make_tuple(
    // {7AB7384D-F5C6-43F9-B13C-80DCC788EE1D}
    GUID { 0x7ab7384d, 0xf5c6, 0x43f9, { 0xb1, 0x3c, 0x80, 0xdc, 0xc7, 0x88, 0xee, 0x1d } },
    std::wstring(L"Composing"),
    TF_DISPLAYATTRIBUTE {
        { TF_CT_NONE, 0 }, // text foreground
        { TF_CT_NONE, 0 }, // text background
        TF_LS_DOT, // underline style
        FALSE, // bold underline
        { TF_CT_NONE, 0 }, // underline color
        TF_ATTR_INPUT // attribute info
    }
);

VietType::DisplayAttributeInfo::DisplayAttributeInfo() {
}

VietType::DisplayAttributeInfo::~DisplayAttributeInfo() {
}

HRESULT VietType::DisplayAttributeInfo::GetGUID(GUID * pguid) {
    *pguid = _guid;
    return S_OK;
}

HRESULT VietType::DisplayAttributeInfo::GetDescription(BSTR * pbstrDesc) {
    *pbstrDesc = SysAllocString(_description.c_str());
    return *pbstrDesc ? S_OK : E_OUTOFMEMORY;
}

HRESULT VietType::DisplayAttributeInfo::GetAttributeInfo(TF_DISPLAYATTRIBUTE * pda) {
    *pda = _attr;
    return S_OK;
}

HRESULT VietType::DisplayAttributeInfo::SetAttributeInfo(const TF_DISPLAYATTRIBUTE * pda) {
    _attr = *pda;
    return S_OK;
}

HRESULT VietType::DisplayAttributeInfo::Reset(void) {
    _attr = _attrOrig;
    return S_OK;
}

void VietType::DisplayAttributeInfo::Initialize(GUID const & guid, std::wstring description, TF_DISPLAYATTRIBUTE attr) {
    _guid = guid;
    _description = description;
    _attr = attr;
    _attrOrig = attr;
}
