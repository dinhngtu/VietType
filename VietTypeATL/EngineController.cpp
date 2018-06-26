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

#include "EngineController.h"

// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
const GUID VietType::GUID_LanguageBarButton_Item = { 0xcca3d390, 0xef1a, 0x4de4, { 0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b } };

VietType::EngineController::EngineController() {
}

VietType::EngineController::~EngineController() {
}

HRESULT VietType::EngineController::OnClick(TfLBIClick click, POINT pt, const RECT *area) {
    if (click == TF_LBI_CLK_LEFT) {
        return ToggleEnabled();
    } else if (click == TF_LBI_CLK_RIGHT) {
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
        if (itemId) {
            return OnMenuSelect(itemId);
        } else {
            return S_OK;
        }
    } else {
        return S_OK;
    }
}

HRESULT VietType::EngineController::InitMenu(ITfMenu * menu) {
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
        DBG_HRESULT_CHECK(hr, L"menu->AddMenuItem failed for %s", buf);
    }
    DestroyMenu(menuSource);

    return S_OK;
}

HRESULT VietType::EngineController::OnMenuSelect(UINT id) {
    switch (id) {
    case ID_TRAY_ABOUT:
        MessageBox(NULL, L"VietType", L"VietType", MB_OK | MB_ICONINFORMATION);
        break;
    }
    return S_OK;
}

HRESULT VietType::EngineController::OnChange(REFGUID rguid) {
    if (IsEqualGUID(rguid, Globals::GUID_KeyEventSink_Compartment_Toggle)) {
        UpdateEnabled();
    }

    return S_OK;
}

HRESULT VietType::EngineController::GetIcon(HICON * hicon) {
    // Windows docs is a liar, icons are mandatory
    if (IsEnabled()) {
        *hicon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_ICONV), IMAGE_ICON, 16, 16, 0));
    } else {
        *hicon = static_cast<HICON>(LoadImage(Globals::dllInstance, MAKEINTRESOURCE(IDI_ICONE), IMAGE_ICON, 16, 16, 0));
    }
    return *hicon ? S_OK : E_FAIL;
}

HRESULT VietType::EngineController::Initialize(
    std::shared_ptr<EngineState> const & engine,
    ITfThreadMgr * threadMgr,
    TfClientId clientid) {

    HRESULT hr;

    _engine = engine;
    _langBarItemMgr = threadMgr;
    _clientid = clientid;

    // deadly but not quite fatal
    SmartComPtr<ITfCompartmentMgr> compartmentMgr;
    hr = threadMgr->GetGlobalCompartment(compartmentMgr.GetAddress());
    if (SUCCEEDED(hr)) {

        hr = compartmentMgr->GetCompartment(Globals::GUID_KeyEventSink_Compartment_Toggle, _compartment.GetAddress());
        if (SUCCEEDED(hr)) {

            SmartComPtr<ITfSource> compartmentSource(_compartment);
            if (!compartmentSource) {
                return E_FAIL;
            }
            hr = _compartmentEventSink.Advise(compartmentSource, this);
            DBG_HRESULT_CHECK(hr, L"%s", L"_compartmentEventSink.Advise failed");

        } else DBG_HRESULT_CHECK(hr, L"%s", L"compartmentMgr->GetCompartment failed");

    } else DBG_HRESULT_CHECK(hr, L"%s", L"threadMgr->GetGlobalCompartment failed");

    hr = InitLanguageBar();
    HRESULT_CHECK_RETURN(hr, L"%s", L"InitLanguageBar failed");

    return S_OK;
}

HRESULT VietType::EngineController::Uninitialize() {
    HRESULT hr;

    hr = UninitLanguageBar();
    DBG_HRESULT_CHECK(hr, L"%s", L"UninitLanguageBar failed");

    hr = _compartmentEventSink.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_compartmentEventSink.Unadvise failed");

    _compartment.Release();

    _langBarItemMgr.Release();
    _engine.reset();

    return S_OK;
}

VietType::EngineState & VietType::EngineController::GetEngine() {
    return *_engine;
}

VietType::EngineState const & VietType::EngineController::GetEngine() const {
    return *_engine;
}

std::shared_ptr<VietType::EngineState> const & VietType::EngineController::GetEngineShared() {
    return _engine;
}

int VietType::EngineController::IsEnabled() const {
    return _engine->Enabled();
}

HRESULT VietType::EngineController::WriteEnabled(int enabled) {
    HRESULT hr;

    hr = CompartmentWriteEnabled(enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentWriteEnabled failed");
    hr = UpdateEnabled();
    HRESULT_CHECK_RETURN(hr, L"%s", L"UpdateEnabled failed");

    return S_OK;
}

HRESULT VietType::EngineController::ToggleEnabled() {
    return WriteEnabled(!_engine->Enabled());
}

HRESULT VietType::EngineController::UpdateEnabled() {
    HRESULT hr;

    int enabled;
    hr = CompartmentReadEnabled(&enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CompartmentReadEnabled failed");
    _engine->Enabled(enabled);
    RefreshButton(_indicatorButton);
    RefreshButton(_langBarButton);

    return S_OK;
}

HRESULT VietType::EngineController::CompartmentReadEnabled(int * pEnabled) {
    HRESULT hr;

    VARIANT v;
    hr = _compartment->GetValue(&v);
    if (hr == S_FALSE) {
        v.vt = VT_I4;
        v.lVal = 1;
        hr = _compartment->SetValue(_clientid, &v);
        *pEnabled = v.lVal;
        DBG_HRESULT_CHECK(hr, L"%s", L"_compartment->SetValue failed");
    } else if (hr == S_OK) {
        if (v.vt != VT_I4) {
            return E_FAIL;
        }
        *pEnabled = v.lVal;
    }

    return hr;
}

HRESULT VietType::EngineController::CompartmentWriteEnabled(int enabled) {
    HRESULT hr;

    VARIANT v;
    v.vt = VT_I4;
    v.lVal = enabled;
    hr = _compartment->SetValue(_clientid, &v);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compartment->SetValue failed");

    return S_OK;
}

HRESULT VietType::EngineController::InitLanguageBar() {
    HRESULT hr;

    hr = InitLangBarItem(
        _indicatorButton,
        Globals::GUID_LBI_INPUTMODE,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription,
        this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"InitLangBarItem failed for _indicatorButton");

    hr = InitLangBarItem(
        _langBarButton,
        VietType::GUID_LanguageBarButton_Item,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription,
        this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"InitLangBarItem failed for _langBarButton");

    return S_OK;
}

HRESULT VietType::EngineController::UninitLanguageBar() {
    HRESULT hr;

    hr = UninitLangBarItem(_indicatorButton);
    DBG_HRESULT_CHECK(hr, L"%s", L"UninitLangBarItem failed for _indicatorButton");
    hr = UninitLangBarItem(_langBarButton);
    DBG_HRESULT_CHECK(hr, L"%s", L"UninitLangBarItem failed for _langBarButton");

    return S_OK;
}

HRESULT VietType::EngineController::InitLangBarItem(
    SmartComObjPtr<LanguageBarButton>& button,
    GUID const & guidItem,
    DWORD style,
    ULONG sort,
    const std::wstring & description,
    ILanguageBarCallbacks * callbacks) {

    HRESULT hr;

    hr = button.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"button.CreateInstance failed");
    hr = button->Initialize(
        guidItem,
        style,
        sort,
        description,
        callbacks);
    HRESULT_CHECK_RETURN(hr, L"%s", L"button->Initialize failed");

    hr = _langBarItemMgr->AddItem(button);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_langBarItemMgr->AddItem failed");

    return S_OK;
}

HRESULT VietType::EngineController::UninitLangBarItem(SmartComObjPtr<LanguageBarButton>& button) {
    if (button) {
        HRESULT hr = _langBarItemMgr->RemoveItem(button);
        DBG_HRESULT_CHECK(hr, L"%s", L"button->RemoveItem failed");

        button->Uninitialize();
        button.Release();
    }

    return S_OK;
}

HMENU VietType::EngineController::GetMenu() {
    HMENU menu = LoadMenu(Globals::dllInstance, MAKEINTRESOURCE(IDR_MENU_TRAY));
    menu = GetSubMenu(menu, 0);

    return menu;
}

HRESULT VietType::EngineController::RefreshButton(SmartComObjPtr<LanguageBarButton>& button) {
    HRESULT hr;

    hr = button->SetText(IsEnabled() ? std::wstring(L"VIE") : std::wstring(L"ENG"));
    HRESULT_CHECK_RETURN(hr, L"%s", L"button->SetText failed");
    hr = button->NotifyUpdate(TF_LBI_ICON);
    HRESULT_CHECK_RETURN(hr, L"%s", L"button->NotifyUpdate failed");

    return hr;
}
