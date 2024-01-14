// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "LanguageBarButton.h"

namespace VietType {

class EngineController;
class LanguageBarButton;

class RefreshableButton : public ILanguageBarCallbacks {
public:
    RefreshableButton() = default;
    RefreshableButton(const RefreshableButton&) = delete;
    RefreshableButton& operator=(const RefreshableButton&) = delete;
    ~RefreshableButton() = default;

    virtual HRESULT Refresh();

    // Inherited via ILanguageBarCallbacks
    virtual HRESULT OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* area) override;
    virtual HRESULT InitMenu(__RPC__in_opt ITfMenu* menu) override;
    virtual HRESULT OnMenuSelect(_In_ UINT id) override;
    virtual HRESULT GetIcon(__RPC__deref_out_opt HICON* hicon) override;
    virtual DWORD GetStatus() override;
    virtual std::wstring GetText() override;
    virtual std::wstring GetTooltipString() override;

    // ensure that langbar init does not read engine state
    _Check_return_ HRESULT Initialize(
        _In_ EngineController* ec,
        _In_ ITfLangBarItemMgr* langBarItemMgr,
        _In_ const GUID& guidItem,
        _In_ DWORD style,
        _In_ ULONG sort,
        _In_ const std::wstring& description);
    HRESULT Uninitialize();

protected:
    // RefreshableButtons are owned by EngineController, so we don't want to hold an owning reference to
    // EngineController here
    EngineController* _controller = nullptr;
    CComPtr<LanguageBarButton> _button;

    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;
};

} // namespace VietType
