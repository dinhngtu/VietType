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

class CompositionManager;
class EngineController;

class KeyHandlerEditSession :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfEditSession {
public:
    KeyHandlerEditSession() = default;
    KeyHandlerEditSession(const KeyHandlerEditSession&) = delete;
    KeyHandlerEditSession& operator=(const KeyHandlerEditSession&) = delete;
    ~KeyHandlerEditSession() = default;

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
        _In_ WPARAM wParam,
        _In_ LPARAM lParam,
        _In_reads_(256) const BYTE* keyState,
        _In_ EngineController* controller);

private:
    HRESULT ComposeKey(_In_ TfEditCookie ec);
    HRESULT Commit(_In_ TfEditCookie ec);

private:
    CComPtr<CompositionManager> _compositionManager;
    CComPtr<ITfContext> _context;
    WPARAM _wParam = 0;
    LPARAM _lParam = 0;
    const BYTE* _keyState = nullptr;
    CComPtr<EngineController> _controller;
};

}
