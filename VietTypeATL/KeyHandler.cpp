// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "KeyHandler.h"
#include "Telex.h"
#include "KeyTranslator.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "EditSessions.h"

namespace VietType {

STDMETHODIMP KeyHandlerEditSession::DoEditSession(_In_ TfEditCookie ec) {
    if (!_compositionManager) {
        return E_FAIL;
    }
    DBG_DPRINT(L"KeyHandler ec = %ld", ec);

    HRESULT hr;

    // recover from situations that terminate the composition without us knowing (e.g. mouse clicks)
    if (!_compositionManager->IsComposing() && _controller->GetEngine().Count()) {
        _controller->GetEngine().Reset();
    }

    if (_wParam == 0) {
        EditSessions::EditCommit(ec, _compositionManager, _context, _controller);
    } else if (IsEditKey(_wParam, _lParam, _keyState)) {
        DBG_DPRINT(L"%d edit key, not eating", _wParam);
        // uneaten, ends composition
        _controller->GetEngine().Reset();
        return _compositionManager->EndCompositionNow(ec);
    } else if (_wParam == VK_ESCAPE) {
        // eaten, revert and end composition
        _controller->GetEngine().Cancel();
        auto str = _controller->GetEngine().Retrieve();
        _controller->GetEngine().Reset();
        hr = _compositionManager->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"_compositionManager->SetCompositionText failed");
        return _compositionManager->EndCompositionNow(ec);
    } else if (IsKeyEaten(&_controller->GetEngine(), _compositionManager->IsComposing(), _wParam, _lParam, _keyState)) {
        // eaten, updates composition
        auto state = PushKey(&_controller->GetEngine(), _wParam, _lParam, _keyState);
        EditSessions::EditNextState(ec, _compositionManager, _context, _controller, state);
    } else if (_wParam == VK_SHIFT) {
        // drop shift
    } else {
        // uneaten, commits
        EditSessions::EditCommit(ec, _compositionManager, _context, _controller);
    }

    return S_OK;
}

HRESULT KeyHandlerEditSession::Initialize(
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState) {

    _compositionManager = compositionManager;
    _context = context;
    _controller = controller;
    _wParam = wParam;
    _lParam = lParam;
    memcpy(_keyState, keyState, ARRAYSIZE(_keyState));
    return S_OK;
}

HRESULT KeyHandlerEditSession::Uninitialize() {
    // since edit sessions are asynchronous, we can't know when the reference to the edit session will die
    // therefore, we don't explicitly uninit the class, leaving it to the destructor when the refcount runs out
    return S_OK;
}

} // namespace VietType
