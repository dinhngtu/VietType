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
#include "CompositionManager.h"
#include "Telex.h"

namespace VietType {

class KeyHandlerEditSession :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfEditSession {
public:
    KeyHandlerEditSession();
    ~KeyHandlerEditSession();

    DECLARE_NOT_AGGREGATABLE(KeyHandlerEditSession)
    BEGIN_COM_MAP(KeyHandlerEditSession)
        COM_INTERFACE_ENTRY(ITfEditSession)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfEditSession
    virtual HRESULT DoEditSession(TfEditCookie ec) override;

    void Initialize(const SmartComObjPtr<CompositionManager>& compositionManager, ITfContext *context, WPARAM wParam, LPARAM lParam, BYTE const *keyState, std::shared_ptr<Telex::TelexEngine> const& engine);

private:
    HRESULT ComposeKey(TfEditCookie ec);
    HRESULT Commit(TfEditCookie ec);

private:
    SmartComObjPtr<CompositionManager> _compositionManager;
    SmartComPtr<ITfContext> _context;
    WPARAM _wParam;
    LPARAM _lParam;
    BYTE const *_keyState;
    std::shared_ptr<Telex::TelexEngine> _engine;

private:
    DISALLOW_COPY_AND_ASSIGN(KeyHandlerEditSession);
};

}
