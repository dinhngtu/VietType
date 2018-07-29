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

#include "LanguageBarButton.h"

// just a random number to identify the sink
const DWORD LanguageBarButtonCookie = 0x5a6fdd5e;

VietType::LanguageBarButton::LanguageBarButton() noexcept {
}

VietType::LanguageBarButton::~LanguageBarButton() {
}

STDMETHODIMP VietType::LanguageBarButton::AdviseSink(__RPC__in REFIID riid, __RPC__in_opt IUnknown* punk, __RPC__out DWORD* pdwCookie) {
    HRESULT hr;

    if (!IsEqualIID(riid, IID_ITfLangBarItemSink)) {
        return CONNECT_E_CANNOTCONNECT;
    }

    if (_itemSink) {
        // only one sink at a time
        return CONNECT_E_ADVISELIMIT;
    }

    hr = punk->QueryInterface(&_itemSink);
    if (FAILED(hr)) {
        _itemSink.Detach();
        return E_NOINTERFACE;
    }

    *pdwCookie = LanguageBarButtonCookie;

    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::UnadviseSink(_In_ DWORD dwCookie) {
    if (dwCookie != LanguageBarButtonCookie || !_itemSink) {
        return CONNECT_E_NOCONNECTION;
    }

    _itemSink.Release();
    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::GetInfo(__RPC__out TF_LANGBARITEMINFO* pInfo) {
    if (!pInfo) {
        return E_INVALIDARG;
    }

    pInfo->clsidService = VietType::Globals::CLSID_TextService;
    pInfo->guidItem = _guidItem;
    pInfo->dwStyle = _style;
    pInfo->ulSort = _sort;
    StringCchCopy(pInfo->szDescription, TF_LBI_DESC_MAXLEN, _description.c_str());

    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::GetStatus(__RPC__out DWORD* pdwStatus) {
    *pdwStatus = _callbacks->GetStatus();
    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::Show(_In_ BOOL fShow) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::LanguageBarButton::GetTooltipString(__RPC__deref_out_opt BSTR* pbstrToolTip) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::LanguageBarButton::OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* prcArea) {
    if (_callbacks) {
        return _callbacks->OnClick(click, pt, prcArea);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::InitMenu(__RPC__in_opt ITfMenu* pMenu) {
    if (!pMenu) {
        return E_INVALIDARG;
    }
    if (_callbacks) {
        return _callbacks->InitMenu(pMenu);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::OnMenuSelect(_In_ UINT wID) {
    if (_callbacks) {
        return _callbacks->OnMenuSelect(wID);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::GetIcon(__RPC__deref_out_opt HICON* phIcon) {
    if (_callbacks) {
        return _callbacks->GetIcon(phIcon);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::GetText(__RPC__deref_out_opt BSTR* pbstrText) {
    *pbstrText = SysAllocString(_callbacks->GetText().c_str());
    return *pbstrText ? S_OK : E_OUTOFMEMORY;
}

_Check_return_ HRESULT VietType::LanguageBarButton::Initialize(
    _In_ const GUID& guidItem,
    _In_ DWORD style,
    _In_ ULONG sort,
    _In_ const std::wstring& description,
    _In_ ILanguageBarCallbacks* callbacks) {
    _guidItem = guidItem;
    _style = style;
    _sort = sort;
    _description = description;
    _callbacks = callbacks;
    return S_OK;
}

HRESULT VietType::LanguageBarButton::NotifyUpdate(_In_ DWORD flags) {
    if (_itemSink) {
        HRESULT hr = _itemSink->OnUpdate(flags);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_itemSink->OnUpdate failed");
    }

    return S_OK;
}

void VietType::LanguageBarButton::Uninitialize() {
    _callbacks = nullptr;
}
