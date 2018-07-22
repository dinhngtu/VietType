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

namespace VietType {

extern const GUID GUID_LanguageBarButton_Item;

class ILanguageBarCallbacks {
public:
    virtual HRESULT OnClick(TfLBIClick click, POINT pt, const RECT* area) = 0;
    virtual HRESULT InitMenu(ITfMenu* menu) = 0;
    virtual HRESULT OnMenuSelect(UINT id) = 0;
    virtual HRESULT GetIcon(HICON* hicon) = 0;
    virtual DWORD GetStatus() = 0;
    virtual std::wstring GetText() = 0;
};

class LanguageBarButton :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfSource,
    public ITfLangBarItemButton {
public:
    LanguageBarButton() noexcept;
    ~LanguageBarButton();

    DECLARE_NOT_AGGREGATABLE(LanguageBarButton)
    BEGIN_COM_MAP(LanguageBarButton)
        COM_INTERFACE_ENTRY(ITfLangBarItemButton)
        COM_INTERFACE_ENTRY(ITfLangBarItem)
        COM_INTERFACE_ENTRY(ITfSource)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfSource
    virtual STDMETHODIMP AdviseSink(REFIID riid, IUnknown* punk, DWORD* pdwCookie) override;
    virtual STDMETHODIMP UnadviseSink(DWORD dwCookie) override;

    // Inherited via ITfLangBarItemButton
    virtual STDMETHODIMP GetInfo(TF_LANGBARITEMINFO* pInfo) override;
    virtual STDMETHODIMP GetStatus(DWORD* pdwStatus) override;
    virtual STDMETHODIMP Show(BOOL fShow) override;
    virtual STDMETHODIMP GetTooltipString(BSTR* pbstrToolTip) override;
    virtual STDMETHODIMP OnClick(TfLBIClick click, POINT pt, const RECT* prcArea) override;
    virtual STDMETHODIMP InitMenu(ITfMenu* pMenu) override;
    virtual STDMETHODIMP OnMenuSelect(UINT wID) override;
    virtual STDMETHODIMP GetIcon(HICON* phIcon) override;
    virtual STDMETHODIMP GetText(BSTR* pbstrText) override;

    HRESULT Initialize(
        const GUID& guidItem,
        DWORD style,
        ULONG sort,
        const std::wstring& description,
        ILanguageBarCallbacks* callbacks);
    HRESULT NotifyUpdate(DWORD flags);
    void Uninitialize();

private:
    GUID _guidItem = { 0 };
    DWORD _style = 0;
    ULONG _sort = 0;
    std::wstring _description;
    ILanguageBarCallbacks* _callbacks = nullptr;

    CComPtr<ITfLangBarItemSink> _itemSink;

private:
    DISALLOW_COPY_AND_ASSIGN(LanguageBarButton);
};

}
