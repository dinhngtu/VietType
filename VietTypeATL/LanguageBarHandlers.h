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

    virtual HRESULT Refresh() = 0;

    _Check_return_ HRESULT Initialize(
        _In_ EngineController* ec,
        _In_ ITfLangBarItemMgr* langBarItemMgr,
        _In_ const GUID& guidItem,
        _In_ DWORD style,
        _In_ ULONG sort,
        _In_ const std::wstring& description);
    HRESULT Uninitialize();

protected:
    // RefreshableButtons are owned by EngineController
    // so we don't want to hold an owning reference to EngineController here
    EngineController* _controller = nullptr;
    CComPtr<LanguageBarButton> _button;

    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;
};

class IndicatorButton : public RefreshableButton {
public:
    IndicatorButton() = default;
    IndicatorButton(const IndicatorButton&) = delete;
    IndicatorButton& operator=(const IndicatorButton&) = delete;
    ~IndicatorButton() = default;

    // Inherited via ILanguageBarCallbacks
    virtual HRESULT OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* area) override;
    virtual HRESULT InitMenu(__RPC__in_opt ITfMenu* menu) override;
    virtual HRESULT OnMenuSelect(_In_ UINT id) override;
    virtual HRESULT GetIcon(__RPC__deref_out_opt HICON* hicon) override;
    virtual DWORD GetStatus() override;
    virtual std::wstring GetText() override;

    // Inherited via RefreshableButton
    virtual HRESULT Refresh() override;
};

class LangBarButton : public RefreshableButton {
public:
    LangBarButton() = default;
    LangBarButton(const LangBarButton&) = delete;
    LangBarButton& operator=(const LangBarButton&) = delete;
    ~LangBarButton() = default;

    // Inherited via ILanguageBarCallbacks
    virtual HRESULT OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* area) override;
    virtual HRESULT InitMenu(__RPC__in_opt ITfMenu* menu) override;
    virtual HRESULT OnMenuSelect(_In_ UINT id) override;
    virtual HRESULT GetIcon(__RPC__deref_out_opt HICON* hicon) override;
    virtual DWORD GetStatus() override;
    virtual std::wstring GetText() override;

    // Inherited via RefreshableButton
    virtual HRESULT Refresh() override;
};

}
