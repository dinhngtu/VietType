// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "LanguageBarButton.h"
#include "EngineController.h"

namespace VietType {

static DWORD SystemUsesLightTheme() {
    CRegKey key;
    DWORD light;
    LSTATUS err = key.Open(
        HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", KEY_QUERY_VALUE);
    if (err != ERROR_SUCCESS) {
        return 0;
    }
    err = key.QueryDWORDValue(L"SystemUsesLightTheme", light);
    if (err != ERROR_SUCCESS) {
        return 0;
    }
    return light;
}

// just a random number to identify the sink
static const DWORD LanguageBarButtonCookie = 0x5a6fdd5e;

STDMETHODIMP LanguageBarButton::AdviseSink(
    __RPC__in REFIID riid, __RPC__in_opt IUnknown* punk, __RPC__out DWORD* pdwCookie) {
    HRESULT hr;

    if (riid != IID_ITfLangBarItemSink) {
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

STDMETHODIMP LanguageBarButton::UnadviseSink(_In_ DWORD dwCookie) {
    if (dwCookie != LanguageBarButtonCookie || !_itemSink) {
        return CONNECT_E_NOCONNECTION;
    }

    _itemSink.Release();
    return S_OK;
}

STDMETHODIMP LanguageBarButton::GetInfo(__RPC__out TF_LANGBARITEMINFO* pInfo) {
    if (!pInfo) {
        return E_INVALIDARG;
    }

    pInfo->clsidService = Globals::CLSID_TextService;
    pInfo->guidItem = _guidItem;
    pInfo->dwStyle = _style;
    pInfo->ulSort = _sort;
    StringCchCopyW(pInfo->szDescription, TF_LBI_DESC_MAXLEN, _description.c_str());

    return S_OK;
}

STDMETHODIMP LanguageBarButton::GetStatus(__RPC__out DWORD* pdwStatus) {
    *pdwStatus = 0;
    return S_OK;
}

STDMETHODIMP LanguageBarButton::Show(_In_ BOOL fShow) {
    return E_NOTIMPL;
}

STDMETHODIMP LanguageBarButton::OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* prcArea) {
    if (click == TF_LBI_CLK_LEFT) {
        return _controller->ToggleUserEnabled();
    }
    return S_OK;
}

STDMETHODIMP LanguageBarButton::InitMenu(__RPC__in_opt ITfMenu* pMenu) {
    if (!pMenu) {
        return E_INVALIDARG;
    }
    return S_OK;
}

STDMETHODIMP LanguageBarButton::OnMenuSelect(_In_ UINT wID) {
    return S_OK;
}

STDMETHODIMP LanguageBarButton::GetIcon(__RPC__deref_out_opt HICON* phIcon) {
    // Windows docs is a liar, icons are mandatory
    if (!Globals::DllInstance) {
        DBG_DPRINT(L"%s", L"cannot obtain instance");
        return E_FAIL;
    }

    DWORD light = SystemUsesLightTheme();
    auto old = SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    auto dpi = GetDpiForSystem();
    int iconx = GetSystemMetricsForDpi(SM_CXSMICON, dpi);
    int icony = GetSystemMetricsForDpi(SM_CYSMICON, dpi);
    SetThreadDpiAwarenessContext(old);
    if (_controller->GetBlocked() == EngineController::BlockedKind::Blocked) {
        *phIcon = static_cast<HICON>(LoadImage(
            Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONXL : IDI_ICONXD), IMAGE_ICON, iconx, icony, 0));
    } else if (_controller->IsEnabled()) {
        *phIcon = static_cast<HICON>(LoadImage(
            Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONVL : IDI_ICONVD), IMAGE_ICON, iconx, icony, 0));
    } else {
        *phIcon = static_cast<HICON>(LoadImage(
            Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONEL : IDI_ICONED), IMAGE_ICON, iconx, icony, 0));
    }
    return *phIcon ? S_OK : E_FAIL;
}

STDMETHODIMP LanguageBarButton::GetText(__RPC__deref_out_opt BSTR* pbstrText) {
    if (!pbstrText) {
        return E_INVALIDARG;
    }
    *pbstrText = SysAllocString(_controller->IsEnabled() ? L"VIE" : L"ENG");
    return *pbstrText ? S_OK : E_OUTOFMEMORY;
}

STDMETHODIMP LanguageBarButton::GetTooltipString(__RPC__deref_out_opt BSTR* pbstrText) {
    if (!pbstrText) {
        return E_INVALIDARG;
    }
    const wchar_t* status = Globals::TextServiceDescription;
    if (_controller->GetBlocked() != EngineController::BlockedKind::Blocked) {
        status = _controller->IsEnabled() ? L"Ti\x1ebfng Vi\x1ec7t" : L"English";
    }
    *pbstrText = SysAllocString(status);
    return *pbstrText ? S_OK : E_OUTOFMEMORY;
}

_Check_return_ HRESULT LanguageBarButton::Initialize(
    _In_ EngineController* ec,
    _In_ ITfLangBarItemMgr* langBarItemMgr,
    _In_ const GUID& guidItem,
    _In_ DWORD style,
    _In_ ULONG sort,
    _In_z_ const wchar_t* description) {
    HRESULT hr;

    _controller = ec;
    _langBarItemMgr = langBarItemMgr;
    _guidItem = guidItem;
    _style = style;
    _sort = sort;
    _description = description;

    hr = _langBarItemMgr->AddItem(this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarItemMgr->AddItem failed");

    return S_OK;
}

HRESULT LanguageBarButton::Uninitialize() {
    HRESULT hr;
    hr = _langBarItemMgr->RemoveItem(this);
    DBG_HRESULT_CHECK(hr, L"%s", L"langBarItemMgr->RemoveItem failed");
    _langBarItemMgr.Release();
    return S_OK;
}

HRESULT LanguageBarButton::NotifyUpdate(_In_ DWORD flags) {
    if (_itemSink) {
        HRESULT hr = _itemSink->OnUpdate(flags);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_itemSink->OnUpdate failed");
    }

    return S_OK;
}

HRESULT LanguageBarButton::Refresh() {
    return NotifyUpdate(TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP);
}

} // namespace VietType
