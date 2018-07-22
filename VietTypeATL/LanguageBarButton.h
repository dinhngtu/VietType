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
    virtual HRESULT OnClick(_In_ TfLBIClick click, _In_ POINT pt, _In_ const RECT* area) = 0;
    virtual HRESULT InitMenu(_In_ ITfMenu* menu) = 0;
    virtual HRESULT OnMenuSelect(_In_ UINT id) = 0;
    virtual HRESULT GetIcon(_Outptr_ HICON* hicon) = 0;
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
    virtual STDMETHODIMP AdviseSink(_In_ REFIID riid, _In_ IUnknown* punk, _Out_ DWORD* pdwCookie) override;
    virtual STDMETHODIMP UnadviseSink(_In_ DWORD dwCookie) override;

    // Inherited via ITfLangBarItemButton
    virtual STDMETHODIMP GetInfo(_Out_ TF_LANGBARITEMINFO* pInfo) override;
    virtual STDMETHODIMP GetStatus(_Out_ DWORD* pdwStatus) override;
    virtual STDMETHODIMP Show(_In_ BOOL fShow) override;
    virtual STDMETHODIMP GetTooltipString(_Outptr_ BSTR* pbstrToolTip) override;
    virtual STDMETHODIMP OnClick(_In_ TfLBIClick click, _In_ POINT pt, _In_ const RECT* prcArea) override;
    virtual STDMETHODIMP InitMenu(_In_ ITfMenu* pMenu) override;
    virtual STDMETHODIMP OnMenuSelect(_In_ UINT wID) override;
    virtual STDMETHODIMP GetIcon(_Outptr_ HICON* phIcon) override;
    virtual STDMETHODIMP GetText(_Outptr_ BSTR* pbstrText) override;

    _Check_return_ HRESULT Initialize(
        _In_ const GUID& guidItem,
        _In_ DWORD style,
        _In_ ULONG sort,
        _In_ const std::wstring& description,
        _In_ ILanguageBarCallbacks* callbacks);
    HRESULT NotifyUpdate(_In_ DWORD flags);
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
