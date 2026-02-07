// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "KeyTranslator.h"

namespace VietType {

HRESULT Context::EditKey(
    _In_ TfEditCookie ec,
    _In_ Context* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState) {
    DBG_DPRINT(L"KeyHandler ec = %ld", ec);

    HRESULT hr;
    Telex::ITelexEngine* engine = context->GetEngine();

    // recover from situations that terminate the composition without us knowing (e.g. mouse clicks)
    if (!context->GetComposition() && engine->Count()) {
        engine->Reset();
    }

    if (wParam == 0) {
        context->EditCommit(ec);
    } else if (IsEditKey(wParam, lParam, keyState)) {
        DBG_DPRINT(L"%d edit key, not eating", wParam);
        // uneaten, ends composition
        engine->Reset();
        return context->EndCompositionNow(ec);
    } else if (wParam == VK_ESCAPE) {
        // eaten, revert and end composition
        engine->Cancel();
        auto str = engine->Retrieve();
        engine->Reset();
        hr = context->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"SetCompositionText failed");
        return context->EndCompositionNow(ec);
    } else if (IsKeyEaten(engine, context->_composition, wParam, lParam, keyState)) {
        // eaten, updates composition
        auto state = PushKey(engine, wParam, lParam, keyState);
        context->EditNextState(ec, state);
    } else if (wParam == VK_SHIFT) {
        // drop shift
    } else {
        // uneaten, commits
        context->EditCommit(ec);
    }

    return S_OK;
}

} // namespace VietType
