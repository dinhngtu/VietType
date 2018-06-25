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

#include "LanguageBar.h"

const DWORD LanguageBarButtonCookie = 0x5a6fdd5e;

// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
const GUID VietType::GUID_LanguageBarButton_Item = { 0xcca3d390, 0xef1a, 0x4de4, { 0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b } };

VietType::LanguageBarButton::LanguageBarButton() {
}

VietType::LanguageBarButton::~LanguageBarButton() {
}

STDMETHODIMP VietType::LanguageBarButton::AdviseSink(REFIID riid, IUnknown * punk, DWORD * pdwCookie) {
    HRESULT hr;

    if (!IsEqualIID(riid, IID_ITfLangBarItemSink)) {
        return CONNECT_E_CANNOTCONNECT;
    }

    if (_itemSink) {
        // only one sink at a time
        return CONNECT_E_ADVISELIMIT;
    }

    hr = punk->QueryInterface(IID_ITfLangBarItemSink, reinterpret_cast<void **>(_itemSink.GetAddress()));
    if (FAILED(hr)) {
        _itemSink.Detach();
        return E_NOINTERFACE;
    }

    *pdwCookie = LanguageBarButtonCookie;

    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::UnadviseSink(DWORD dwCookie) {
    if (dwCookie != LanguageBarButtonCookie || !_itemSink) {
        return CONNECT_E_NOCONNECTION;
    }

    _itemSink.Release();
    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::GetInfo(TF_LANGBARITEMINFO * pInfo) {
    if (!pInfo) {
        return E_INVALIDARG;
    }

    pInfo->clsidService = VietType::Globals::CLSID_TextService;
    pInfo->guidItem = _guidItem;
    pInfo->dwStyle = TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY;
    pInfo->ulSort = _sort;
    StringCchCopy(pInfo->szDescription, TF_LBI_DESC_MAXLEN, _description.c_str());

    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::GetStatus(DWORD * pdwStatus) {
    *pdwStatus = _status;
    return S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::Show(BOOL fShow) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::LanguageBarButton::GetTooltipString(BSTR * pbstrToolTip) {
    return E_NOTIMPL;
}

STDMETHODIMP VietType::LanguageBarButton::OnClick(TfLBIClick click, POINT pt, const RECT * prcArea) {
    if (_callbacks) {
        return _callbacks->OnClick(click);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::InitMenu(ITfMenu * pMenu) {
    if (_callbacks) {
        return _callbacks->InitMenu(pMenu);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::OnMenuSelect(UINT wID) {
    if (_callbacks) {
        return _callbacks->OnMenuSelect(wID);
    } else {
        return E_FAIL;
    }
}

STDMETHODIMP VietType::LanguageBarButton::GetIcon(HICON * phIcon) {
    // Windows docs is a liar, icons are mandatory
    *phIcon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_IMELOGO), IMAGE_ICON, 16, 16, 0));
    return *phIcon ? E_FAIL : S_OK;
}

STDMETHODIMP VietType::LanguageBarButton::GetText(BSTR * pbstrText) {
    *pbstrText = SysAllocString(_text.c_str());
    return *pbstrText ? S_OK : E_OUTOFMEMORY;
}

HRESULT VietType::LanguageBarButton::Initialize(GUID const & guidItem, ULONG sort, std::wstring const & description, ILanguageBarCallbacks *callbacks) {
    _guidItem = guidItem;
    _sort = sort;
    _description = description;
    _callbacks = callbacks;
    return S_OK;
}

HRESULT VietType::LanguageBarButton::NotifyUpdate(DWORD flags) {
    if (_itemSink) {
        HRESULT hr = _itemSink->OnUpdate(flags);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_itemSink->OnUpdate failed");
    }

    return S_OK;
}

void VietType::LanguageBarButton::Uninitialize() {
    _callbacks = nullptr;
}

HRESULT VietType::LanguageBarButton::SetStatus(DWORD flags) {
    _status = flags;
    return NotifyUpdate(TF_LBI_STATUS);
}

HRESULT VietType::LanguageBarButton::SetText(std::wstring text) {
    _text = text;
    return NotifyUpdate(TF_LBI_TEXT);
}
