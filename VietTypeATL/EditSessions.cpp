// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"

namespace VietType {
namespace EditSessions {

HRESULT EditNextState(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller,
    _In_ Telex::TelexStates state) {
    HRESULT hr;

    DBG_DPRINT(L"");

    switch (state) {
    case Telex::TelexStates::Valid: {
        if (controller->GetEngine().Count()) {
            auto str = controller->GetEngine().Peek();
            hr = compositionManager->EnsureCompositionText(ec, context, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"_compositionManager->EnsureCompositionText failed");
        } else {
            // backspace returns Valid on an empty buffer
            controller->GetEngine().Reset();
            // EndComposition* will not empty composition text so we have to do it manually
            hr = compositionManager->EmptyCompositionText(ec);
            HRESULT_CHECK_RETURN(hr, L"_compositionManager->EmptyCompositionText failed");
            hr = compositionManager->EndCompositionNow(ec);
            HRESULT_CHECK_RETURN(hr, L"_compositionManager->EndCompositionNow failed");
        }
        break;
    }

    case Telex::TelexStates::Invalid: {
        assert(controller->GetEngine().Count() > 0);
        auto str = controller->GetEngine().RetrieveRaw();
        hr = compositionManager->EnsureCompositionText(ec, context, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"_compositionManager->EnsureCompositionText failed");
        break;
    }

    default:
        DBG_DPRINT(L"PushKey returned unexpected value");
        assert(0);
        break;
    }

    return S_OK;
}

HRESULT EditCommit(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller) {
    HRESULT hr = S_OK;

    if (compositionManager->IsComposing()) {
        auto txstate = controller->GetEngine().Commit();
        if (txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid) {
            auto str = controller->GetEngine().Retrieve();
            hr = compositionManager->SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"_compositionManager->EnsureCompositionText failed");
        } else {
            assert(txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid);
            hr = E_FAIL;
        }
    }

    controller->GetEngine().Reset();
    compositionManager->EndCompositionNow(ec);

    return S_OK;
}

} // namespace EditSessions
} // namespace VietType
