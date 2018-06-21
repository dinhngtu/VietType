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

#include "KeyHandler.h"
#include "Telex.h"

VietType::KeyHandlerEditSession::KeyHandlerEditSession() {
}

VietType::KeyHandlerEditSession::~KeyHandlerEditSession() {
}

HRESULT VietType::KeyHandlerEditSession::DoEditSession(TfEditCookie ec) {
    assert((bool)_compositionManager);
    DBG_DPRINT(L"%s", L"entering key handler session");

    if (_wParam == 0) {
        Commit(ec);
    } else if (Telex::IsEditKey(_wParam, _lParam, _keyState)) {
        return _compositionManager->EndCompositionNow(ec);
    } else if (Telex::EngineWantsKey(_compositionManager->IsComposing(), _wParam, _lParam, _keyState)) {
        ComposeKey(ec);
    } else if (_wParam == VK_SHIFT) {
        // drop shift
    } else {
        Commit(ec);
    }

    return S_OK;
}

void VietType::KeyHandlerEditSession::Initialize(
    const SmartComObjPtr<CompositionManager>& compositionManager,
    ITfContext *context,
    WPARAM wParam,
    LPARAM lParam,
    BYTE const * keyState,
    std::shared_ptr<Telex::TelexEngine> const& engine) {

    _compositionManager = compositionManager;
    _context = context;
    _wParam = wParam;
    _lParam = lParam;
    _keyState = keyState;
    _engine = engine;
}

HRESULT VietType::KeyHandlerEditSession::ComposeKey(TfEditCookie ec) {
    HRESULT hr;

    switch (Telex::PushKey(*_engine, _wParam, _lParam, _keyState)) {
    case Telex::TELEX_STATES::VALID: {
        if (_engine->Count()) {
            if (!_compositionManager->IsComposing()) {
                hr = _compositionManager->StartComposition(_context);
                HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->StartComposition failed");
            }

            auto str = _engine->Peek();
            hr = _compositionManager->SetCompositionText(ec, &str[0], (LONG)str.length());
            DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->SetCompositionText failed");
        } else {
            // backspace only returns VALID on an empty buffer
            hr = _compositionManager->EndCompositionNow(ec);
            HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->EndCompositionNow failed");
        }
        break;
    }

    case Telex::TELEX_STATES::INVALID: {
        assert(_engine->Count() > 0);
        assert(_compositionManager->IsComposing());
        auto str = _engine->RetrieveInvalid();
        hr = _compositionManager->SetCompositionText(ec, &str[0], (LONG)str.length());
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->SetCompositionText failed");
    }

    default:
        DPRINT(L"%s", L"PushKey returned unexpected value");
        assert(0);
        break;
    }

    return S_OK;
}

HRESULT VietType::KeyHandlerEditSession::Commit(TfEditCookie ec) {
    HRESULT hr;

    switch (_engine->Commit()) {
    case Telex::TELEX_STATES::COMMITTED: {
        assert(_engine->Count() > 0);
        auto str = _engine->Retrieve();
        hr = _compositionManager->SetCompositionText(ec, &str[0], (LONG)str.length());
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->SetCompositionText failed");
    }

    case Telex::TELEX_STATES::COMMITTED_INVALID: {
        assert(_engine->Count() > 0);
        auto str = _engine->RetrieveInvalid();
        hr = _compositionManager->SetCompositionText(ec, &str[0], (LONG)str.length());
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->SetCompositionText failed");
    }

    default:
        DPRINT(L"%s", L"Commit returned unexpected value");
        assert(0);
        break;
    }

    return S_OK;
}
