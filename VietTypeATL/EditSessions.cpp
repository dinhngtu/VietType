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
            DBG_HRESULT_CHECK(hr, L"_contextManager->EnsureCompositionText failed");
        } else {
            // backspace returns Valid on an empty buffer
            GetEngine()->Reset();
            // EndComposition* will not empty composition text so we have to do it manually
            hr = EmptyCompositionText(ec);
            HRESULT_CHECK_RETURN(hr, L"_contextManager->EmptyCompositionText failed");
            hr = EndCompositionNow(ec);
            HRESULT_CHECK_RETURN(hr, L"_contextManager->EndCompositionNow failed");
        }
        break;
    }

    case Telex::TelexStates::Invalid: {
        assert(GetEngine()->Count() > 0);
        auto str = GetEngine()->RetrieveRaw();
        hr = EnsureCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
        DBG_HRESULT_CHECK(hr, L"_contextManager->EnsureCompositionText failed");
        break;
    }

    default:
        DBG_DPRINT(L"PushKey returned unexpected value");
        assert(0);
        break;
    }

    return S_OK;
}

HRESULT Context::EditCommit(_In_ TfEditCookie ec, _In_ wchar_t appendChar) {
    HRESULT hr = S_OK;

    if (GetComposition()) {
        auto txstate = GetEngine()->Commit();
        if (txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid) {
            auto str = GetEngine()->Retrieve();
            hr = SetCompositionText(ec, &str[0], static_cast<LONG>(str.length()));
            DBG_HRESULT_CHECK(hr, L"_contextManager->EnsureCompositionText failed");

            // push the committed text out of the composition range
            CComPtr<ITfRange> compRange;
            if (SUCCEEDED(_composition->GetRange(&compRange))) {
                CComPtr<ITfRange> newStart;
                if (SUCCEEDED(compRange->Clone(&newStart)) && SUCCEEDED(newStart->Collapse(ec, TF_ANCHOR_END))) {
                    hr = _composition->ShiftStart(ec, newStart);
                    DBG_HRESULT_CHECK(hr, L"_composition->ShiftStart failed");

                    TF_SELECTION sel;
                    sel.range = newStart;
                    sel.style.ase = TF_AE_NONE;
                    sel.style.fInterimChar = FALSE;
                    hr = _context->SetSelection(ec, 1, &sel);
                    DBG_HRESULT_CHECK(hr, L"_context->SetSelection failed");
                }
            }
        } else {
            assert(txstate == Telex::TelexStates::Committed || txstate == Telex::TelexStates::CommittedInvalid);
            hr = E_FAIL;
        }
    }

    GetEngine()->Reset();
    EndCompositionNow(ec);

    if (appendChar != L'\0') {
        DBG_DPRINT(L"InsertTextAtSelection char=%lc", appendChar);

        CComPtr<ITfInsertAtSelection> insertAtSel;
        if (SUCCEEDED(_context->QueryInterface(&insertAtSel))) {
            CComPtr<ITfRange> insertedRange;
            hr = insertAtSel->InsertTextAtSelection(ec, 0, &appendChar, 1, &insertedRange);
            DBG_HRESULT_CHECK(hr, L"InsertTextAtSelection failed");
        }
    }

    return S_OK;
}

} // namespace VietType
