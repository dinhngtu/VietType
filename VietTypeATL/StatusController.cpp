// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "StatusController.h"
#include "ContextManager.h"
#include "LanguageBarButton.h"

namespace VietType {

// {CCA3D390-EF1A-4DE4-B2FF-B6BC76D68C3B}
static const GUID GUID_LanguageBarButton_Item = {
    0xcca3d390, 0xef1a, 0x4de4, {0xb2, 0xff, 0xb6, 0xbc, 0x76, 0xd6, 0x8c, 0x3b}};

_Check_return_ HRESULT StatusController::Initialize(_In_ ContextManager* parent, _In_ ITfThreadMgr* threadMgr) {
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
