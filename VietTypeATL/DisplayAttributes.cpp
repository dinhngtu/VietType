// SPDX-License-Identifier: GPL-3.0-only

#include "DisplayAttributes.h"

namespace VietType {

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
