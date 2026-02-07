// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "StatusController.h"
#include "CompositionManager.h"
#include "LanguageBarButton.h"

namespace VietType {

// {B31B741B-63CE-413A-9B5A-D2B69C695A78}
static const GUID GUID_SettingsCompartment_Toggle = {
    0xb31b741b, 0x63ce, 0x413a, {0x9b, 0x5a, 0xd2, 0xb6, 0x9c, 0x69, 0x5a, 0x78}};
// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
static const GUID GUID_LanguageBarButton_Item = {
    0xcca3d390, 0xef1a, 0x4de4, {0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b}};
// {B2FBD2E7-922F-4996-BE77-21085B91A8F0}
static const GUID GUID_SystemNotifyCompartment = {
    0xb2fbd2e7, 0x922f, 0x4996, {0xbe, 0x77, 0x21, 0x8, 0x5b, 0x91, 0xa8, 0xf0}};

_Check_return_ HRESULT StatusController::Initialize(_In_ CompositionManager* parent, _In_ ITfThreadMgr* threadMgr) {
    HRESULT hr;

    _parent = parent;

    hr = threadMgr->QueryInterface(&_langBarItemMgr);
    HRESULT_CHECK_RETURN(hr, L"threadMgr->QueryInterface failed");

    hr = InitLanguageBar();
    HRESULT_CHECK_RETURN(hr, L"InitLanguageBar failed");

    return S_OK;
}

HRESULT StatusController::Uninitialize() {
    HRESULT hr;

    hr = UninitLanguageBar();
    DBG_HRESULT_CHECK(hr, L"UninitLanguageBar failed");

    _langBarItemMgr.Release();

    return S_OK;
}

HRESULT StatusController::UpdateStatus(bool isEnabled, bool isBlocked) {
    _enabled = isEnabled;
    _blocked = isBlocked;
    _indicatorButton->Refresh();
    _langBarButton->Refresh();
    return S_OK;
}

void StatusController::ToggleUserEnabled() {
    _parent->ToggleUserEnabled();
}

_Check_return_ HRESULT StatusController::InitLanguageBar() {
    HRESULT hr;

    hr = CreateInitialize(
        &_indicatorButton,
        this,
        _langBarItemMgr,
        Globals::GUID_LBI_INPUTMODE,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"_indicatorButton->Initialize failed");

    hr = CreateInitialize(
        &_langBarButton,
        this,
        _langBarItemMgr,
        GUID_LanguageBarButton_Item,
        TF_LBI_STYLE_BTN_BUTTON | TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY,
        0,
        Globals::TextServiceDescription);
    HRESULT_CHECK_RETURN(hr, L"_langBarButton->Initialize failed");

    return S_OK;
}

HRESULT StatusController::UninitLanguageBar() {
    if (_langBarButton)
        _langBarButton->Uninitialize();
    _langBarButton.Release();

    if (_indicatorButton)
        _indicatorButton->Uninitialize();
    _indicatorButton.Release();

    return S_OK;
}

} // namespace VietType
