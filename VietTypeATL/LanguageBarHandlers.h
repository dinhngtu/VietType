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

class RefreshableButton : public ILanguageBarCallbacks {
public:
    explicit RefreshableButton(EngineController *ec);
    virtual ~RefreshableButton() {
    }

    virtual HRESULT Refresh() {
        return E_NOTIMPL;
    }

    HRESULT Initialize(
        ITfLangBarItemMgr *langBarItemMgr,
        GUID const & guidItem,
        DWORD style,
        ULONG sort,
        const std::wstring & description);
    void Uninitialize();

protected:
    EngineController *_controller;
    CComPtr<LanguageBarButton> _button;

    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;

private:
    DISALLOW_COPY_AND_ASSIGN(RefreshableButton);
};

class IndicatorButton : public RefreshableButton {
public:
    explicit IndicatorButton(EngineController *ec);

    // Inherited via ILanguageBarCallbacks
    virtual HRESULT OnClick(TfLBIClick click, POINT pt, const RECT * area) override;
    virtual HRESULT InitMenu(ITfMenu * menu) override;
    virtual HRESULT OnMenuSelect(UINT id) override;
    virtual HRESULT GetIcon(HICON * hicon) override;
    virtual DWORD GetStatus() override;
    virtual std::wstring GetText() override;

    // Inherited via RefreshableButton
    virtual HRESULT Refresh() override;

private:
    DISALLOW_COPY_AND_ASSIGN(IndicatorButton);
};

class LangBarButton : public RefreshableButton {
public:
    explicit LangBarButton(EngineController *ec);

    // Inherited via ILanguageBarCallbacks
    virtual HRESULT OnClick(TfLBIClick click, POINT pt, const RECT * area) override;
    virtual HRESULT InitMenu(ITfMenu * menu) override;
    virtual HRESULT OnMenuSelect(UINT id) override;
    virtual HRESULT GetIcon(HICON * hicon) override;
    virtual DWORD GetStatus() override;
    virtual std::wstring GetText() override;

    // Inherited via RefreshableButton
    virtual HRESULT Refresh() override;

private:
    DISALLOW_COPY_AND_ASSIGN(LangBarButton);
};

}
