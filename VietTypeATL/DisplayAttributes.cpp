// SPDX-License-Identifier: GPL-3.0-only

#include "DisplayAttributes.h"

namespace VietType {

const std::tuple<GUID, std::wstring, TF_DISPLAYATTRIBUTE> ComposingAttributeData = std::make_tuple(
    // {7AB7384D-F5C6-43F9-B13C-80DCC788EE1D}
    GUID{0x7ab7384d, 0xf5c6, 0x43f9, {0xb1, 0x3c, 0x80, 0xdc, 0xc7, 0x88, 0xee, 0x1d}},
    std::wstring(L"Composing"),
    TF_DISPLAYATTRIBUTE{
        {TF_CT_NONE, 0}, // text foreground
        {TF_CT_NONE, 0}, // text background
        TF_LS_DOT,       // underline style
        FALSE,           // bold underline
        {TF_CT_NONE, 0}, // underline color
        TF_ATTR_INPUT    // attribute info
    });

STDMETHODIMP DisplayAttributeInfo::GetGUID(__RPC__out GUID* pguid) {
    *pguid = _guid;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfo::GetDescription(__RPC__deref_out_opt BSTR* pbstrDesc) {
    *pbstrDesc = SysAllocString(_description.c_str());
    return *pbstrDesc ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP DisplayAttributeInfo::GetAttributeInfo(__RPC__out TF_DISPLAYATTRIBUTE* pda) {
    *pda = _attr;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfo::SetAttributeInfo(__RPC__in const TF_DISPLAYATTRIBUTE* pda) {
    _attr = *pda;
    return S_OK;
}

STDMETHODIMP DisplayAttributeInfo::Reset(void) {
    _attr = _attrOrig;
    return S_OK;
}

HRESULT DisplayAttributeInfo::Initialize(
    _In_ const GUID& guid, _In_ std::wstring description, _In_ TF_DISPLAYATTRIBUTE attr) {
    _guid = guid;
    _description = description;
    _attr = attr;
    _attrOrig = attr;
    return S_OK;
}

} // namespace VietType
