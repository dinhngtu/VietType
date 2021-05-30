// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class ILanguageBarCallbacks {
public:
    virtual HRESULT OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* prcArea) = 0;
    virtual HRESULT InitMenu(__RPC__in_opt ITfMenu* menu) = 0;
    virtual HRESULT OnMenuSelect(_In_ UINT id) = 0;
    virtual HRESULT GetIcon(__RPC__deref_out_opt HICON* hicon) = 0;
    virtual DWORD GetStatus() = 0;
    virtual std::wstring GetText() = 0;
    virtual std::wstring GetTooltipString() = 0;
};

class LanguageBarButton :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfSource,
    public ITfLangBarItemButton {
public:
    LanguageBarButton() = default;
    LanguageBarButton(const LanguageBarButton&) = delete;
    LanguageBarButton& operator=(const LanguageBarButton&) = delete;
    ~LanguageBarButton() = default;

    DECLARE_NOT_AGGREGATABLE(LanguageBarButton)
    BEGIN_COM_MAP(LanguageBarButton)
        COM_INTERFACE_ENTRY(ITfLangBarItemButton)
        COM_INTERFACE_ENTRY(ITfLangBarItem)
        COM_INTERFACE_ENTRY(ITfSource)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfSource
    virtual STDMETHODIMP AdviseSink(__RPC__in REFIID riid, __RPC__in_opt IUnknown* punk, __RPC__out DWORD* pdwCookie) override;
    virtual STDMETHODIMP UnadviseSink(_In_ DWORD dwCookie) override;

    // Inherited via ITfLangBarItemButton
    virtual STDMETHODIMP GetInfo(__RPC__out TF_LANGBARITEMINFO* pInfo) override;
    virtual STDMETHODIMP GetStatus(__RPC__out DWORD* pdwStatus) override;
    virtual STDMETHODIMP Show(_In_ BOOL fShow) override;
    virtual STDMETHODIMP GetTooltipString(__RPC__deref_out_opt BSTR* pbstrToolTip) override;
    virtual STDMETHODIMP OnClick(_In_ TfLBIClick click, _In_ POINT pt, __RPC__in const RECT* prcArea) override;
    virtual STDMETHODIMP InitMenu(__RPC__in_opt ITfMenu* pMenu) override;
    virtual STDMETHODIMP OnMenuSelect(_In_ UINT wID) override;
    virtual STDMETHODIMP GetIcon(__RPC__deref_out_opt HICON* phIcon) override;
    virtual STDMETHODIMP GetText(__RPC__deref_out_opt BSTR* pbstrText) override;

    _Check_return_ HRESULT Initialize(
        _In_ const GUID& guidItem,
        _In_ DWORD style,
        _In_ ULONG sort,
        _In_ const std::wstring& description,
        _In_ ILanguageBarCallbacks* callbacks);
    HRESULT NotifyUpdate(_In_ DWORD flags);
    HRESULT Uninitialize();

private:
    GUID _guidItem = { 0 };
    DWORD _style = 0;
    ULONG _sort = 0;
    std::wstring _description;
    ILanguageBarCallbacks* _callbacks = nullptr;

    CComPtr<ITfLangBarItemSink> _itemSink;
};

}
