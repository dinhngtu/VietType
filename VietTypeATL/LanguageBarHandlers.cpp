#include "LanguageBarHandlers.h"
#include "EngineController.h"

HMENU GetMenu() {
    assert(VietType::Globals::dllInstance);
    HMENU menu = LoadMenu(VietType::Globals::dllInstance, MAKEINTRESOURCE(IDR_MENU_TRAY));
    menu = GetSubMenu(menu, 0);

    return menu;
}

int PopMenu(POINT pt, const RECT * area) {
    HMENU menu = GetMenu();
    if (!menu) {
        DBG_DPRINT(L"%s", L"load menu failed");
    }
    UINT flags = TPM_NONOTIFY | TPM_RETURNCMD;
    if (GetSystemMetrics(SM_MENUDROPALIGNMENT)) {
        flags |= TPM_RIGHTALIGN;
    }
    int itemId = TrackPopupMenuEx(menu, flags, pt.x, pt.y, /* doesn't matter */ GetFocus(), NULL);
    DestroyMenu(menu);
    return itemId;
}

HRESULT CopyTfMenu(ITfMenu *menu) {
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
        std::vector<wchar_t> buf(mii.cch);
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

        hr = menu->AddMenuItem(
            mii.wID,
            tfFlags,
            tfBitmap,
            NULL,
            &buf[0],
            static_cast<ULONG>(buf.size()),
            NULL);
        DBG_HRESULT_CHECK(hr, L"%s", L"menu->AddMenuItem failed");
    }
    DestroyMenu(menuSource);

    return S_OK;
}

HRESULT OnMenuSelectAll(UINT id) {
    switch (id) {
    case ID_TRAY_ABOUT:
        MessageBox(NULL, L"VietType", L"VietType", MB_OK | MB_ICONINFORMATION);
        break;
    }
    return S_OK;
}

////////////////////////////////////////////////////////////////////////////////
// RefreshableButton
////////////////////////////////////////////////////////////////////////////////

VietType::RefreshableButton::RefreshableButton(EngineController * ec) {
    _ec = ec;
}

HRESULT VietType::RefreshableButton::Initialize(ITfLangBarItemMgr *langBarItemMgr, GUID const & guidItem, DWORD style, ULONG sort, const std::wstring & description) {
    HRESULT hr;

    _langBarItemMgr = langBarItemMgr;

    hr = _button.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_button.CreateInstance failed");
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

VietType::IndicatorButton::IndicatorButton(EngineController * ec) : RefreshableButton(ec) {
}

HRESULT VietType::IndicatorButton::OnClick(TfLBIClick click, POINT pt, const RECT * area) {
    if (click == TF_LBI_CLK_LEFT) {
        return _ec->ToggleEnabled();
    } else if (click == TF_LBI_CLK_RIGHT) {
        int itemId = PopMenu(pt, area);
        if (itemId) {
            return OnMenuSelectAll(itemId);
        }
    }
    return S_OK;
}

HRESULT VietType::IndicatorButton::InitMenu(ITfMenu * menu) {
    CopyTfMenu(menu);
    return S_OK;
}

HRESULT VietType::IndicatorButton::OnMenuSelect(UINT id) {
    return OnMenuSelectAll(id);
}

HRESULT VietType::IndicatorButton::GetIcon(HICON * hicon) {
    // Windows docs is a liar, icons are mandatory
    assert(Globals::dllInstance);
    if (_ec->IsEnabled()) {
        *hicon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_ICONV), IMAGE_ICON, 16, 16, 0));
    } else {
        *hicon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_ICONE), IMAGE_ICON, 16, 16, 0));
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
    return _ec->IsEnabled() ? std::wstring(L"VIE") : std::wstring(L"ENG");
}

////////////////////////////////////////////////////////////////////////////////
// LangBarButton
////////////////////////////////////////////////////////////////////////////////

VietType::LangBarButton::LangBarButton(EngineController * ec) : RefreshableButton(ec) {
}

HRESULT VietType::LangBarButton::OnClick(TfLBIClick click, POINT pt, const RECT * area) {
    if (click == TF_LBI_CLK_LEFT) {
        return _ec->ToggleEnabled();
    } else if (click == TF_LBI_CLK_RIGHT) {
        int itemId = PopMenu(pt, area);
        if (itemId) {
            return OnMenuSelectAll(itemId);
        }
    }
    return S_OK;
}

HRESULT VietType::LangBarButton::InitMenu(ITfMenu * menu) {
    CopyTfMenu(menu);
    return S_OK;
}

HRESULT VietType::LangBarButton::OnMenuSelect(UINT id) {
    return OnMenuSelectAll(id);
}

HRESULT VietType::LangBarButton::GetIcon(HICON * hicon) {
    // Windows docs is a liar, icons are mandatory
    assert(Globals::dllInstance);
    if (_ec->IsEnabled()) {
        *hicon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_ICONV), IMAGE_ICON, 16, 16, 0));
    } else {
        *hicon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_ICONE), IMAGE_ICON, 16, 16, 0));
    }
    return *hicon ? S_OK : E_FAIL;
}

DWORD VietType::LangBarButton::GetStatus() {
    return 0;
}

std::wstring VietType::LangBarButton::GetText() {
    return _ec->IsEnabled() ? std::wstring(L"VIE") : std::wstring(L"ENG");
}

HRESULT VietType::LangBarButton::Refresh() {
    HRESULT hr;

    hr = _button->NotifyUpdate(TF_LBI_ICON);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_button->NotifyUpdate failed");

    return hr;
}
