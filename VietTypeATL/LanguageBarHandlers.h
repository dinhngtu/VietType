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
    EngineController * _ec;
    SmartComObjPtr<LanguageBarButton> _button;

    SmartComPtr<ITfLangBarItemMgr> _langBarItemMgr;

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
