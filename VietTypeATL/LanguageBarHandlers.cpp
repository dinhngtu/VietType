// SPDX-License-Identifier: GPL-3.0-only

#include "LanguageBarHandlers.h"
#include "EngineController.h"
#include "Version.h"

namespace VietType {

static DWORD GetSystemLightTheme() {
    CRegKey key;
    DWORD light = 0;
    LSTATUS err = key.Open(
        HKEY_CURRENT_USER, L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize", KEY_QUERY_VALUE);
    if (err == ERROR_SUCCESS) {
        err = key.QueryDWORDValue(L"SystemUsesLightTheme", light);
        if (err != ERROR_SUCCESS) {
            light = 0;
        }
    } else {
        light = 0;
    }
    return light;
}

////////////////////////////////////////////////////////////////////////////////
// RefreshableButton
////////////////////////////////////////////////////////////////////////////////

_Check_return_ HRESULT RefreshableButton::Initialize(
    _In_ EngineController* ec,
    _In_ ITfLangBarItemMgr* langBarItemMgr,
    _In_ const GUID& guidItem,
    _In_ DWORD style,
    _In_ ULONG sort,
    _In_ const std::wstring& description) {
    HRESULT hr;

    _controller = ec;
    _langBarItemMgr = langBarItemMgr;

    hr = CreateInitialize(&_button, guidItem, style, sort, description, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(&_button) failed");
    hr = _langBarItemMgr->AddItem(_button);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarItemMgr->AddItem failed");

    return S_OK;
}

HRESULT RefreshableButton::Uninitialize() {
    HRESULT hr;
    hr = _langBarItemMgr->RemoveItem(_button);
    DBG_HRESULT_CHECK(hr, L"%s", L"langBarItemMgr->RemoveItem failed");
    _button->Uninitialize();
    _button.Release();
    _langBarItemMgr.Release();
    return S_OK;
}

HRESULT RefreshableButton::OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* area) {
    if (click == TF_LBI_CLK_LEFT) {
        return _controller->ToggleUserEnabled();
    }
    return S_OK;
}

HRESULT RefreshableButton::InitMenu(__RPC__in_opt ITfMenu* menu) {
    return S_OK;
}

HRESULT RefreshableButton::OnMenuSelect(_In_ UINT id) {
    return S_OK;
}

HRESULT RefreshableButton::GetIcon(__RPC__deref_out_opt HICON* hicon) {
    // Windows docs is a liar, icons are mandatory
    if (!Globals::DllInstance) {
        DBG_DPRINT(L"%s", L"cannot obtain instance");
        return E_FAIL;
    }

    DWORD light = GetSystemLightTheme();
    int iconx = GetSystemMetrics(SM_CXSMICON);
    int icony = GetSystemMetrics(SM_CYSMICON);
    if (_controller->GetBlocked() == EngineController::BlockedKind::Blocked) {
        *hicon = static_cast<HICON>(LoadImage(
            Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONXL : IDI_ICONXD), IMAGE_ICON, iconx, icony, 0));
    } else if (_controller->IsEnabled()) {
        *hicon = static_cast<HICON>(LoadImage(
            Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONVL : IDI_ICONVD), IMAGE_ICON, iconx, icony, 0));
    } else {
        *hicon = static_cast<HICON>(LoadImage(
            Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONEL : IDI_ICONED), IMAGE_ICON, iconx, icony, 0));
    }
    return *hicon ? S_OK : E_FAIL;
}

DWORD RefreshableButton::GetStatus() {
    return 0;
}

std::wstring RefreshableButton::GetText() {
    return _controller->IsEnabled() ? std::wstring(L"VIE") : std::wstring(L"ENG");
}

std::wstring RefreshableButton::GetTooltipString() {
    if (_controller->GetBlocked() == EngineController::BlockedKind::Blocked) {
        return std::wstring(L"Paused");
    } else {
        return _controller->IsEnabled() ? std::wstring(L"Vietnamese") : std::wstring(L"English");
    }
}

HRESULT RefreshableButton::Refresh() {
    HRESULT hr;

    hr = _button->NotifyUpdate(TF_LBI_ICON | TF_LBI_TEXT | TF_LBI_TOOLTIP);
    HRESULT_CHECK_RETURN(hr, L"%s", L"button->NotifyUpdate failed");

    return hr;
}

} // namespace VietType
