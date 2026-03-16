// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "KeyTranslator.h"

namespace VietType {

HRESULT Context::EditKey(_In_ TfEditCookie ec, _In_ Context* context, _In_ KeyResult keyResult, _In_ wchar_t push) {
    DBG_DPRINT(L"KeyHandler ec = %ld %s '%c'", ec, GetKeyResult(keyResult), push ? push : L'?');

    HRESULT hr;
    Telex::ITelexEngine* engine = context->GetEngine();

    CComPtr<ITfComposition> composition;
    hr = context->GetComposition(ec, &composition);
    HRESULT_CHECK_RETURN(hr, L"context->GetComposition failed");

    // recover from situations that terminate the composition without us knowing (e.g. mouse clicks)
    if (!composition && engine->Count()) {
        DBG_DPRINT("resetting");
        engine->Reset();
    }

    switch (keyResult) {
    case KeyResult::BreakingCharacter:
        return context->DoEditNextState(ec, composition, engine->Commit(), push);
    case KeyResult::Character:
        return context->DoEditNextState(ec, composition, engine->PushChar(push), L'\0');
    case KeyResult::Backspace:
        return context->DoEditNextState(ec, composition, engine->Backspace(), L'\0');
    case KeyResult::Escape: {
        return context->DoEditNextState(ec, composition, engine->Cancel(), L'\0');
    }
    case KeyResult::NotEaten:
    case KeyResult::NotEatenEndComposition:
    default:
        engine->Reset();
        return context->EndCompositionNow(ec, composition);
    }
}

} // namespace VietType
