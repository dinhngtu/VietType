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
    KeyHandlerEditSession() noexcept;
    ~KeyHandlerEditSession();

    DECLARE_NOT_AGGREGATABLE(KeyHandlerEditSession)
    BEGIN_COM_MAP(KeyHandlerEditSession)
        COM_INTERFACE_ENTRY(ITfEditSession)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfEditSession
    virtual STDMETHODIMP DoEditSession(TfEditCookie ec) override;

    void Initialize(
        CompositionManager* compositionManager,
        ITfContext* context,
        WPARAM wParam,
        LPARAM lParam,
        BYTE const* keyState,
        EngineController* controller);

private:
    HRESULT ComposeKey(TfEditCookie ec);
    HRESULT Commit(TfEditCookie ec);

private:
    CComPtr<CompositionManager> _compositionManager;
    CComPtr<ITfContext> _context;
    WPARAM _wParam = 0;
    LPARAM _lParam = 0;
    const BYTE* _keyState = nullptr;
    CComPtr<EngineController> _controller;

private:
    DISALLOW_COPY_AND_ASSIGN(KeyHandlerEditSession);
};

}
