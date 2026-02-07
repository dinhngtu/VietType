// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"

namespace VietType {

HRESULT Context::EditNextState(_In_ TfEditCookie ec, _In_ Telex::TelexStates state) {
    HRESULT hr;

    DBG_DPRINT(L"");

    switch (state) {
    case Telex::TelexStates::Valid: {
        if (GetEngine()->Count()) {
            auto str = GetEngine()->Peek();
            hr = EnsureCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"_compositionManager->EnsureCompositionText failed");
        } else {
            // backspace returns Valid on an empty buffer
            GetEngine()->Reset();
            // EndComposition* will not empty composition text so we have to do it manually
            hr = EmptyCompositionText(ec);
            HRESULT_CHECK_RETURN(hr, L"_compositionManager->EmptyCompositionText failed");
            hr = EndCompositionNow(ec);
            HRESULT_CHECK_RETURN(hr, L"_compositionManager->EndCompositionNow failed");
        }
        break;
    }

    case Telex::TelexStates::Invalid: {
        assert(GetEngine()->Count() > 0);
        auto str = GetEngine()->RetrieveRaw();
        hr = EnsureCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
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

HRESULT Context::EditCommit(_In_ TfEditCookie ec) {
    HRESULT hr = S_OK;

    if (GetComposition()) {
        auto txstate = GetEngine()->Commit();
        if (txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid) {
            auto str = GetEngine()->Retrieve();
            hr = SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"_compositionManager->EnsureCompositionText failed");
        } else {
            assert(txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid);
            hr = E_FAIL;
        }
    }

    GetEngine()->Reset();
    EndCompositionNow(ec);

    return S_OK;
}

} // namespace VietType
