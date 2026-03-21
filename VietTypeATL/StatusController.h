// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class ContextManager;
class LanguageBarButton;

class ATL_NO_VTABLE StatusController : public CComObjectRootEx<CComSingleThreadModel> {
public:
    StatusController() = default;
    StatusController(const StatusController&) = delete;
    StatusController& operator=(const StatusController&) = delete;
    ~StatusController() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(StatusController)
    BEGIN_COM_MAP(StatusController)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    _Check_return_ HRESULT Initialize(_In_ ContextManager* parent, _In_ ITfThreadMgr* threadMgr);
    HRESULT Uninitialize();
    void FinalRelease() {
        Uninitialize();
    }

    HRESULT UpdateStatus(bool isEnabled, bool isBlocked);

    // for langbars
    constexpr bool IsEnabled() const {
        return _enabled;
    }
    constexpr bool IsBlocked() const {
        return _blocked;
    }
    void ToggleUserEnabled();

private:
    _Check_return_ HRESULT InitLanguageBar();
    HRESULT UninitLanguageBar();

private:
    ContextManager* _parent = nullptr;

    CComPtr<ITfLangBarItemMgr> _langBarItemMgr;
    CComPtr<LanguageBarButton> _indicatorButton;
    CComPtr<LanguageBarButton> _langBarButton;

    // cached
    bool _enabled = false;
    bool _blocked = true;
};

} // namespace VietType
