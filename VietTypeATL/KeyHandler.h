// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class CompositionManager;
class EngineController;

class KeyHandlerEditSession : public CComObjectRootEx<CComSingleThreadModel>, public ITfEditSession {
public:
    KeyHandlerEditSession() = default;
    KeyHandlerEditSession(const KeyHandlerEditSession&) = delete;
    KeyHandlerEditSession& operator=(const KeyHandlerEditSession&) = delete;
    ~KeyHandlerEditSession() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(KeyHandlerEditSession)
    BEGIN_COM_MAP(KeyHandlerEditSession)
    COM_INTERFACE_ENTRY(ITfEditSession)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfEditSession
    virtual STDMETHODIMP DoEditSession(_In_ TfEditCookie ec) override;

    HRESULT Initialize(
        _In_ CompositionManager* compositionManager,
        _In_ ITfContext* context,
        _In_ EngineController* controller,
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _In_reads_(256) const BYTE* keyState);
    // dummy method
    HRESULT Uninitialize();

private:
    CComPtr<CompositionManager> _compositionManager;
    CComPtr<ITfContext> _context;
    CComPtr<EngineController> _controller;
    WPARAM _wParam = 0;
    LPARAM _lParam = 0;
    BYTE _keyState[256] = {0};
};

} // namespace VietType
