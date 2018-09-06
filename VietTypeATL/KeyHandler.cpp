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
#include "CompositionManager.h"
#include "EngineController.h"

namespace VietType {

STDMETHODIMP KeyHandlerEditSession::DoEditSession(_In_ TfEditCookie ec) {
    assert(_compositionManager);
    DBG_DPRINT(L"ec = %ld", ec);

    HRESULT hr;

    // recover from situations that terminate the composition without us knowing (e.g. mouse clicks)
    if (!_compositionManager->IsComposing() && _controller->GetEngine().Count()) {
        _controller->GetEngine().Reset();
    }

    if (_wParam == 0) {
        Commit(ec);
    } else if (Telex::IsEditKey(_wParam, _lParam, _keyState)) {
        DBG_DPRINT(L"%d not edit key, not eating", _wParam);
        // uneaten, ends composition
        _controller->GetEngine().Reset();
        return _compositionManager->EndCompositionNow(ec);
    } else if (_wParam == VK_ESCAPE) {
        // eaten, revert and end composition
        auto str = _controller->GetEngine().RetrieveInvalid();
        _controller->GetEngine().Reset();
        hr = _compositionManager->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->SetCompositionText failed");
        return _compositionManager->EndCompositionNow(ec);
    } else if (Telex::IsKeyEaten(_compositionManager->IsComposing(), _wParam, _lParam, _keyState)) {
        // eaten, updates composition
        ComposeKey(ec);
    } else if (_wParam == VK_SHIFT) {
        // drop shift
    } else {
        // uneaten, commits
        Commit(ec);
    }

    return S_OK;
}

HRESULT KeyHandlerEditSession::Initialize(
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ EngineController* controller) {

    // since edit sessions are asynchronous, we can't know when the reference to the edit session will die
    // therefore, we don't explicitly uninit the class, leaving it to the destructor when the refcount runs out
    _compositionManager = compositionManager;
    _context = context;
    _wParam = wParam;
    _lParam = lParam;
    _keyState = keyState;
    _controller = controller;
    return S_OK;
}

HRESULT KeyHandlerEditSession::Uninitialize() {
    return S_OK;
}

HRESULT KeyHandlerEditSession::ComposeKey(_In_ TfEditCookie ec) {
    HRESULT hr;

    DBG_DPRINT(L"%s", L"");

    switch (Telex::PushKey(_controller->GetEngine(), _wParam, _lParam, _keyState)) {
    case Telex::TelexStates::Valid: {
        if (_controller->GetEngine().Count()) {
            auto str = _controller->GetEngine().Peek();
            hr = _compositionManager->EnsureCompositionText(ec, _context, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EnsureCompositionText failed");
        } else {
            // backspace returns Valid on an empty buffer
            _controller->GetEngine().Reset();
            // EndComposition* will not empty composition text so we have to do it manually
            hr = _compositionManager->EmptyCompositionText(ec);
            HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->EmptyCompositionText failed");
            hr = _compositionManager->EndCompositionNow(ec);
            HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->EndCompositionNow failed");
        }
        break;
    }

    case Telex::TelexStates::Invalid: {
        assert(_controller->GetEngine().Count() > 0);
        auto str = _controller->GetEngine().RetrieveInvalid();
        hr = _compositionManager->EnsureCompositionText(ec, _context, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EnsureCompositionText failed");
        break;
    }

    default:
        DBG_DPRINT(L"%s", L"PushKey returned unexpected value");
        assert(0);
        break;
    }

    return S_OK;
}

HRESULT KeyHandlerEditSession::Commit(_In_ TfEditCookie ec) {
    HRESULT hr;

    DBG_DPRINT(L"%s", L"");

    if (!_compositionManager->IsComposing()) {
        goto exit;
    }

    switch (_controller->GetEngine().Commit()) {
    case Telex::TelexStates::Committed: {
        auto str = _controller->GetEngine().Retrieve();
        hr = _compositionManager->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EnsureCompositionText failed");
        break;
    }

    case Telex::TelexStates::CommittedInvalid: {
        auto str = _controller->GetEngine().RetrieveInvalid();
        hr = _compositionManager->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EnsureCompositionText failed");
        break;
    }

    default:
        DBG_DPRINT(L"%s", L"Commit returned unexpected value");
        assert(0);
        break;
    }

exit:
    _controller->GetEngine().Reset();
    _compositionManager->EndCompositionNow(ec);

    return S_OK;
}

}
