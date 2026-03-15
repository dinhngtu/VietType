// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "KeyTranslator.h"

namespace VietType {

HRESULT Context::EditKey(_In_ TfEditCookie ec, _In_ Context* context, _In_ KeyResult keyResult, _In_ wchar_t push) {
    DBG_DPRINT(L"KeyHandler ec = %ld %s '%c'", ec, GetKeyResult(keyResult), push);

    HRESULT hr;
    Telex::ITelexEngine* engine = context->GetEngine();

    // recover from situations that terminate the composition without us knowing (e.g. mouse clicks)
    if (!context->GetComposition() && engine->Count()) {
        DBG_DPRINT("resetting");
        engine->Reset();
    }

    switch (keyResult) {
    case KeyResult::NotEatenEndComposition:
        engine->Reset();
        return context->EndCompositionNow(ec);
    case KeyResult::BreakingCharacter:
    case KeyResult::ComposingCharacter:
        auto state = engine->PushChar(push);
        return context->EditNextState(ec, state);
    case KeyResult::ComposingBackspace:
        auto state = engine->Backspace();
        return context->EditNextState(ec, state);
    case KeyResult::ComposingEscape: {
        engine->Cancel();
        auto str = engine->Retrieve();
        engine->Reset();
        hr = context->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"SetCompositionText failed");
        return context->EndCompositionNow(ec);
    }
    default:
        return E_NOTIMPL;
    }
}

} // namespace VietType
