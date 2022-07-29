// SPDX-License-Identifier: GPL-3.0-only

#include "KeyHandler.h"
#include "Telex.h"
#include "CompositionManager.h"
#include "EngineController.h"

namespace VietType {

STDMETHODIMP KeyHandlerEditSession::DoEditSession(_In_ TfEditCookie ec) {
    if (!_compositionManager) {
        return E_FAIL;
    }
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
        auto str = _controller->GetEngine().RetrieveRaw();
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

    _compositionManager = compositionManager;
    _context = context;
    _wParam = wParam;
    _lParam = lParam;
    _keyState = keyState;
    _controller = controller;
    return S_OK;
}

HRESULT KeyHandlerEditSession::Uninitialize() {
    // since edit sessions are asynchronous, we can't know when the reference to the edit session will die
    // therefore, we don't explicitly uninit the class, leaving it to the destructor when the refcount runs out
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
        auto str = _controller->GetEngine().RetrieveRaw();
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
    HRESULT hr = S_OK;

    if (_compositionManager->IsComposing()) {
        auto txstate = _controller->GetEngine().Commit();
        if (txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid) {
            auto str = _controller->GetEngine().Retrieve();
            hr = _compositionManager->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"%s", L"_compositionManager->EnsureCompositionText failed");
        } else {
            assert(txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid);
            hr = E_FAIL;
        }
    }

    _controller->GetEngine().Reset();
    _compositionManager->EndCompositionNow(ec);

    return S_OK;
}

} // namespace VietType
