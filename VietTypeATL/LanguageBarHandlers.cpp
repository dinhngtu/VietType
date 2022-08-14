// SPDX-License-Identifier: GPL-3.0-only

#include "LanguageBarHandlers.h"
#include "EngineController.h"
#include "Version.h"

namespace VietType {

class SharedMenu {
public:
    SharedMenu() = delete;
    SharedMenu(const SharedMenu&) = delete;
    SharedMenu& operator=(const SharedMenu&) = delete;
    ~SharedMenu() = default;

    explicit SharedMenu(HMENU menu) : _menu(menu, DestroyMenu) {
    }

    HMENU GetTrayMenu() {
        if (!_menu) {
            return NULL;
        }
        return GetSubMenu(_menu.get(), 0);
    }

    operator bool() const {
        return !!_menu;
    }

private:
    UNIQUE_HANDLE_DECL(HMENU, DestroyMenu) _menu;
};

static SharedMenu GetSharedMenu() {
    if (!Globals::DllInstance) {
        DBG_DPRINT(L"%s", L"cannot obtain instance");
        return SharedMenu(NULL);
    }
    HMENU menu = LoadMenu(Globals::DllInstance, MAKEINTRESOURCE(IDR_MENU_TRAY));
    if (!menu) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetSharedMenu failed");
        return SharedMenu(NULL);
    }
    return SharedMenu(menu);
}

static int PopMenu(POINT pt, const RECT* area) {
    auto menu = GetSharedMenu();
    if (!menu) {
        DBG_DPRINT(L"%s", L"load menu failed");
        return 0;
    }
    auto trayMenu = menu.GetTrayMenu();
    if (!trayMenu) {
        DBG_DPRINT(L"%s", L"load tray menu failed");
        return 0;
    }
    UINT flags = TPM_NONOTIFY | TPM_RETURNCMD;
    if (GetSystemMetrics(SM_MENUDROPALIGNMENT)) {
        flags |= TPM_RIGHTALIGN;
    }
    int itemId = TrackPopupMenuEx(trayMenu, flags, pt.x, pt.y, /* doesn't matter */ GetFocus(), NULL);
    return itemId;
}

static HRESULT CopyTfMenu(_In_ ITfMenu* menu) {
    HRESULT hr;

    auto menuSource = GetSharedMenu();
    if (!menuSource) {
        return E_FAIL;
    }
    auto trayMenu = menuSource.GetTrayMenu();
    if (!trayMenu) {
        return E_FAIL;
    }
    for (int i = 0; i < GetMenuItemCount(trayMenu); i++) {
        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.dwTypeData = NULL;
        mii.fMask = MIIM_STRING;
        if (!GetMenuItemInfo(trayMenu, i, TRUE, &mii)) {
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetMenuItemInfo failed");
        }

        mii.cch++;
        mii.fMask = MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
        // the vector used here can't outlive the copy function, that's why the function has to do the item addition by
        // itself
        std::vector<WCHAR> buf(mii.cch);
        mii.dwTypeData = &buf[0];
        if (!GetMenuItemInfo(trayMenu, i, TRUE, &mii)) {
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetMenuItemInfo failed");
        }

        DWORD tfFlags = 0;
        if (mii.fState & MFS_CHECKED) {
            tfFlags |= TF_LBMENUF_CHECKED;
        }
        if (mii.fType & MFT_SEPARATOR) {
            tfFlags |= TF_LBMENUF_SEPARATOR;
        }
        if (mii.fType & MFT_RADIOCHECK) {
            tfFlags |= TF_LBMENUF_RADIOCHECKED;
        }
        if (mii.fState & MFS_GRAYED) {
            tfFlags |= TF_LBMENUF_GRAYED;
        }

        auto tfBitmap = UNIQUE_HANDLE(static_cast<HBITMAP>(NULL), DeleteObject);
        if (mii.hbmpItem && mii.hbmpItem != reinterpret_cast<HBITMAP>(-1)) {
            tfBitmap.reset(static_cast<HBITMAP>(CopyImage(mii.hbmpItem, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE)));
        }

        // hbmpMask is not supposed to be NULL here but it still works anyway so we suppress the warning
#pragma warning(suppress : 6387)
        hr = menu->AddMenuItem(mii.wID, tfFlags, tfBitmap.get(), NULL, &buf[0], static_cast<ULONG>(buf.size()), NULL);
        DBG_HRESULT_CHECK(hr, L"%s", L"menu->AddMenuItem failed");
    }

    return S_OK;
}

static HRESULT OnMenuSelectAll(_In_ UINT id, _In_ EngineController* controller) {
    switch (id) {
    case 0:
        return S_OK;

    case ID_TRAY_ABOUT: {
        LPCWSTR aboutFormatString = nullptr;
        // LoadString will return a read-only pointer to the loaded resource string, no need to free
        if (!LoadString(Globals::DllInstance, IDS_ABOUTSTRING, reinterpret_cast<LPWSTR>(&aboutFormatString), 0)) {
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"LoadString failed");
        }
        assert(aboutFormatString);
        if (!aboutFormatString) {
            DBG_DPRINT(L"%s", L"cannot get aboutFormatString");
            return E_FAIL;
        }

        LPWSTR ptext = nullptr;
        std::array<DWORD_PTR, 5> args = {PRODUCTVERSION_TUPLE, reinterpret_cast<DWORD_PTR>(VCS_REVISION)};
        if (!FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                aboutFormatString,
                0,
                0,
                reinterpret_cast<LPWSTR>(&ptext),
                0,
                reinterpret_cast<va_list*>(&args[0]))) {
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"FormatMessage failed");
        }

        auto text = UNIQUE_HANDLE(ptext, LocalFree);
        if (!MessageBox(NULL, text.get(), Globals::TextServiceDescription.c_str(), MB_OK | MB_ICONINFORMATION)) {
            WINERROR_GLE_RETURN_HRESULT(L"%s", L"MessageBox failed");
        }
        break;
    }
    }
    return S_OK;
}

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
    } else if (click == TF_LBI_CLK_RIGHT) {
        int itemId = PopMenu(pt, area);
        if (itemId) {
            return OnMenuSelectAll(itemId, _controller);
        }
    }
    return S_OK;
}

HRESULT RefreshableButton::InitMenu(__RPC__in_opt ITfMenu* menu) {
    if (!menu) {
        return E_INVALIDARG;
    }
    CopyTfMenu(menu);
    return S_OK;
}

HRESULT RefreshableButton::OnMenuSelect(_In_ UINT id) {
    return OnMenuSelectAll(id, _controller);
}

HRESULT RefreshableButton::GetIcon(__RPC__deref_out_opt HICON* hicon) {
    // Windows docs is a liar, icons are mandatory
    if (!Globals::DllInstance) {
        DBG_DPRINT(L"%s", L"cannot obtain instance");
        return E_FAIL;
    }

    DWORD light = GetSystemLightTheme();
    if (_controller->GetBlocked() == EngineController::BlockedKind::Blocked) {
        *hicon = static_cast<HICON>(
            LoadImage(Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONXL : IDI_ICONXD), IMAGE_ICON, 16, 16, 0));
    } else if (_controller->IsEnabled()) {
        *hicon = static_cast<HICON>(
            LoadImage(Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONVL : IDI_ICONVD), IMAGE_ICON, 16, 16, 0));
    } else {
        *hicon = static_cast<HICON>(
            LoadImage(Globals::DllInstance, MAKEINTRESOURCE(light ? IDI_ICONEL : IDI_ICONED), IMAGE_ICON, 16, 16, 0));
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
