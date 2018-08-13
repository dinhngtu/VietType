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

#include "LanguageBarHandlers.h"
#include "EngineController.h"
#include "Version.h"

HMENU GetMenu() {
    assert(VietType::Globals::DllInstance);
    HMENU menu = LoadMenu(VietType::Globals::DllInstance, MAKEINTRESOURCE(IDR_MENU_TRAY));
    menu = GetSubMenu(menu, 0);

    return menu;
}

int PopMenu(POINT pt, const RECT* area) {
    HMENU menu = GetMenu();
    if (!menu) {
        DBG_DPRINT(L"%s", L"load menu failed");
        return 0;
    }
    UINT flags = TPM_NONOTIFY | TPM_RETURNCMD;
    if (GetSystemMetrics(SM_MENUDROPALIGNMENT)) {
        flags |= TPM_RIGHTALIGN;
    }
    int itemId = TrackPopupMenuEx(menu, flags, pt.x, pt.y, /* doesn't matter */ GetFocus(), NULL);
    DestroyMenu(menu);
    return itemId;
}

HRESULT CopyTfMenu(_In_ ITfMenu* menu) {
    HRESULT hr;

    HMENU menuSource = GetMenu();
    for (int i = 0; i < GetMenuItemCount(menuSource); i++) {
        MENUITEMINFO mii;
        mii.cbSize = sizeof(MENUITEMINFO);
        mii.dwTypeData = NULL;
        mii.fMask = MIIM_STRING;
        if (!GetMenuItemInfo(menuSource, i, TRUE, &mii)) {
            WINERROR_RETURN_HRESULT(L"%s", L"GetMenuItemInfo failed");
        }

        mii.cch++;
        mii.fMask = MIIM_BITMAP | MIIM_CHECKMARKS | MIIM_FTYPE | MIIM_ID | MIIM_STATE | MIIM_STRING;
        // the vector used here can't outlive the copy function, that's why the function has to do the item addition by itself
        std::vector<WCHAR> buf(mii.cch);
        mii.dwTypeData = &buf[0];
        if (!GetMenuItemInfo(menuSource, i, TRUE, &mii)) {
            WINERROR_RETURN_HRESULT(L"%s", L"GetMenuItemInfo failed");
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

        HBITMAP tfBitmap = NULL;
        if (mii.hbmpItem && mii.hbmpItem != reinterpret_cast<HBITMAP>(-1)) {
            tfBitmap = static_cast<HBITMAP>(CopyImage(mii.hbmpItem, IMAGE_BITMAP, 0, 0, LR_DEFAULTSIZE));
        }
        if (!tfBitmap) {
            break;
        }

#pragma warning(push)
#pragma warning(disable: 6387)
        // hbmpMask is not supposed to be NULL here but it still works anyway so we disabled the warning
        hr = menu->AddMenuItem(
            mii.wID,
            tfFlags,
            tfBitmap,
            NULL,
            &buf[0],
            static_cast<ULONG>(buf.size()),
            NULL);
#pragma warning(pop)
        DBG_HRESULT_CHECK(hr, L"%s", L"menu->AddMenuItem failed");

        DeleteObject(tfBitmap);
    }
    DestroyMenu(menuSource);

    return S_OK;
}

HRESULT OnMenuSelectAll(_In_ UINT id) {
    switch (id) {
    case 0:
        return S_OK;
    case ID_TRAY_ABOUT: {
        LPCWSTR aboutFormatString = nullptr;
        // LoadString will return a read-only pointer to the loaded resource string, no need to free
        if (!LoadString(VietType::Globals::DllInstance, IDS_ABOUTSTRING, reinterpret_cast<LPWSTR>(&aboutFormatString), 0)) {
            WINERROR_RETURN_HRESULT(L"%s", L"LoadString failed");
        }
        assert(aboutFormatString);

        LPCWSTR text = nullptr;
        auto version = VietType::ReadVersionData();
        std::array<DWORD_PTR, 5> args = {
            std::get<0>(version),
            std::get<1>(version),
            std::get<2>(version),
            std::get<3>(version),
            reinterpret_cast<DWORD_PTR>(VCS_REVISION)
        };
        if (!FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ARGUMENT_ARRAY,
            aboutFormatString,
            0,
            0,
            reinterpret_cast<LPWSTR>(&text),
            0,
            reinterpret_cast<va_list*>(&args[0]))) {
            WINERROR_RETURN_HRESULT(L"%s", L"FormatMessage failed");
        }

        if (!MessageBox(NULL, text, VietType::Globals::TextServiceDescription.c_str(), MB_OK | MB_ICONINFORMATION)) {
            WINERROR_RETURN_HRESULT(L"%s", L"MessageBox failed");
        }
        LocalFree((HLOCAL)text);
        break;
    }
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// RefreshableButton
////////////////////////////////////////////////////////////////////////////////

_Check_return_ HRESULT VietType::RefreshableButton::Initialize(
    _In_ EngineController* ec,
    _In_ ITfLangBarItemMgr* langBarItemMgr,
    _In_ const GUID& guidItem,
    _In_ DWORD style,
    _In_ ULONG sort,
    _In_ const std::wstring& description) {
    HRESULT hr;

    _controller = ec;
    _langBarItemMgr = langBarItemMgr;

    hr = CreateInstance2(&_button);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&_button) failed");
    hr = _button->Initialize(
        guidItem,
        style,
        sort,
        description,
        this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_button->Initialize failed");
    hr = _langBarItemMgr->AddItem(_button);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarItemMgr->AddItem failed");

    return S_OK;
}

void VietType::RefreshableButton::Uninitialize() {
    HRESULT hr;
    hr = _langBarItemMgr->RemoveItem(_button);
    DBG_HRESULT_CHECK(hr, L"%s", L"langBarItemMgr->RemoveItem failed");
    _button->Uninitialize();
    _button.Release();
    _langBarItemMgr.Release();
}

////////////////////////////////////////////////////////////////////////////////
// IndicatorButton
////////////////////////////////////////////////////////////////////////////////

HRESULT VietType::IndicatorButton::OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* area) {
    if (click == TF_LBI_CLK_LEFT) {
        return _controller->ToggleUserEnabled();
    } else if (click == TF_LBI_CLK_RIGHT) {
        int itemId = PopMenu(pt, area);
        if (itemId) {
            return OnMenuSelectAll(itemId);
        }
    }
    return S_OK;
}

HRESULT VietType::IndicatorButton::InitMenu(__RPC__in_opt ITfMenu* menu) {
    if (!menu) {
        return E_INVALIDARG;
    }
    CopyTfMenu(menu);
    return S_OK;
}

HRESULT VietType::IndicatorButton::OnMenuSelect(_In_ UINT id) {
    return OnMenuSelectAll(id);
}

HRESULT VietType::IndicatorButton::GetIcon(__RPC__deref_out_opt HICON* hicon) {
    // Windows docs is a liar, icons are mandatory
    assert(Globals::DllInstance);
    if (_controller->GetBlocked() == EngineController::BlockedKind::Blocked) {
        *hicon = static_cast<HICON>(LoadImage(Globals::DllInstance, MAKEINTRESOURCE(IDI_ICONX), IMAGE_ICON, 16, 16, 0));
    } else if (_controller->IsEnabled()) {
        *hicon = static_cast<HICON>(LoadImage(Globals::DllInstance, MAKEINTRESOURCE(IDI_ICONV), IMAGE_ICON, 16, 16, 0));
    } else {
        *hicon = static_cast<HICON>(LoadImage(Globals::DllInstance, MAKEINTRESOURCE(IDI_ICONE), IMAGE_ICON, 16, 16, 0));
    }
    return *hicon ? S_OK : E_FAIL;
}

HRESULT VietType::IndicatorButton::Refresh() {
    HRESULT hr;

    hr = _button->NotifyUpdate(TF_LBI_ICON);
    HRESULT_CHECK_RETURN(hr, L"%s", L"button->NotifyUpdate failed");

    return hr;
}

DWORD VietType::IndicatorButton::GetStatus() {
    return 0;
}

std::wstring VietType::IndicatorButton::GetText() {
    return _controller->IsEnabled() ? std::wstring(L"VIE") : std::wstring(L"ENG");
}

////////////////////////////////////////////////////////////////////////////////
// LangBarButton
////////////////////////////////////////////////////////////////////////////////

HRESULT VietType::LangBarButton::OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* area) {
    if (click == TF_LBI_CLK_LEFT) {
        return _controller->ToggleUserEnabled();
    } else if (click == TF_LBI_CLK_RIGHT) {
        int itemId = PopMenu(pt, area);
        if (itemId) {
            return OnMenuSelectAll(itemId);
        }
    }
    return S_OK;
}

HRESULT VietType::LangBarButton::InitMenu(__RPC__in_opt ITfMenu* menu) {
    if (!menu) {
        return E_INVALIDARG;
    }
    CopyTfMenu(menu);
    return S_OK;
}

HRESULT VietType::LangBarButton::OnMenuSelect(_In_ UINT id) {
    return OnMenuSelectAll(id);
}

HRESULT VietType::LangBarButton::GetIcon(__RPC__deref_out_opt HICON* hicon) {
    // Windows docs is a liar, icons are mandatory
    assert(Globals::DllInstance);
    if (_controller->GetBlocked() == EngineController::BlockedKind::Blocked) {
        *hicon = static_cast<HICON>(LoadImage(Globals::DllInstance, MAKEINTRESOURCE(IDI_ICONX), IMAGE_ICON, 16, 16, 0));
    } else if (_controller->IsEnabled()) {
        *hicon = static_cast<HICON>(LoadImage(Globals::DllInstance, MAKEINTRESOURCE(IDI_ICONV), IMAGE_ICON, 16, 16, 0));
    } else {
        *hicon = static_cast<HICON>(LoadImage(Globals::DllInstance, MAKEINTRESOURCE(IDI_ICONE), IMAGE_ICON, 16, 16, 0));
    }
    return *hicon ? S_OK : E_FAIL;
}

DWORD VietType::LangBarButton::GetStatus() {
    return 0;
}

std::wstring VietType::LangBarButton::GetText() {
    return _controller->IsEnabled() ? std::wstring(L"VIE") : std::wstring(L"ENG");
}

HRESULT VietType::LangBarButton::Refresh() {
    HRESULT hr;

    hr = _button->NotifyUpdate(TF_LBI_ICON);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_button->NotifyUpdate failed");

    return hr;
}
