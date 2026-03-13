// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"

namespace VietType {

HRESULT Context::StartCompositionNow(_In_ TfEditCookie ec) {
    HRESULT hr;

    if (_composition) {
        return TF_E_LOCKED;
    }

    CComPtr<ITfCompositionSink> compositionSink;
    hr = QueryInterface2(this, &compositionSink);
    HRESULT_CHECK_RETURN(hr, L"this->QueryInterface failed");

    CComPtr<ITfInsertAtSelection> insertAtSelection;
    hr = _context->QueryInterface(&insertAtSelection);
    HRESULT_CHECK_RETURN(hr, L"context->QueryInterface failed");

    CComPtr<ITfRange> insertRange;
    hr = insertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &insertRange);
    HRESULT_CHECK_RETURN(hr, L"insertAtSelection->InsertTextAtSelection failed");

    CComPtr<ITfContextComposition> contextComposition;
    hr = _context->QueryInterface(&contextComposition);
    HRESULT_CHECK_RETURN(hr, L"context->QueryInterface failed");

    ITfComposition* composition;
    hr = contextComposition->StartComposition(ec, insertRange, compositionSink, &composition);
    if (SUCCEEDED(hr)) {
        _composition = composition;

        TF_SELECTION sel;
        sel.range = insertRange;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = _context->SetSelection(ec, 1, &sel);
        DBG_HRESULT_CHECK(hr, L"context->SetSelection failed");
    } else {
        HRESULT_CHECK_RETURN(hr, L"contextComposition->StartComposition failed")
    }

    return S_OK;
}

HRESULT Context::EndCompositionNow(_In_ TfEditCookie ec) {
    HRESULT hr;

    DBG_DPRINT(L"ending composition");

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        if (SUCCEEDED(hr)) {
            if (_displayAtom != TF_INVALID_GUIDATOM) {
                hr = ClearRangeDisplayAttribute(ec, range);
                DBG_HRESULT_CHECK(hr, L"ClearRangeDisplayAttribute failed");
            }

            hr = MoveCaretToEnd(ec);
            DBG_HRESULT_CHECK(hr, L"MoveCaretToEnd failed");

            hr = _composition->EndComposition(ec);
            DBG_HRESULT_CHECK(hr, L"_composition->EndComposition failed");
        } else {
            HRESULT_CHECK(hr, L"_composition->GetRange failed");
        }

        _composition.Release();
    }

    return S_OK;
}

HRESULT Context::SetCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length) {
    HRESULT hr;

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"_composition->GetRange failed");

        hr = range->SetText(ec, 0, str, length);
        HRESULT_CHECK_RETURN(hr, L"range->SetText failed");

        hr = SetRangeDisplayAttribute(ec, range);
        HRESULT_CHECK_RETURN(hr, L"SetRangeDisplayAttribute failed");

        hr = MoveCaretToEnd(ec);
        DBG_HRESULT_CHECK(hr, L"MoveCaretToEnd failed");
    }

    return S_OK;
}

HRESULT Context::EmptyCompositionText(_In_ TfEditCookie ec) {
    HRESULT hr;

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"composition->GetRange failed");

        hr = range->SetText(ec, 0, nullptr, 0);
        HRESULT_CHECK_RETURN(hr, L"range->SetText failed");
    }

    return S_OK;
}

HRESULT Context::MoveCaretToEnd(_In_ TfEditCookie ec) {
    HRESULT hr;

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"_composition->GetRange failed");

        CComPtr<ITfRange> rangeClone;
        hr = range->Clone(&rangeClone);
        HRESULT_CHECK_RETURN(hr, L"range->Clone failed");

        hr = rangeClone->Collapse(ec, TF_ANCHOR_END);
        HRESULT_CHECK_RETURN(hr, L"rangeClone->Collapse failed");

        TF_SELECTION sel;
        sel.range = rangeClone;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = _context->SetSelection(ec, 1, &sel);
        HRESULT_CHECK_RETURN(hr, L"_context->SetSelection failed");
    }

    return S_OK;
}

HRESULT Context::EnsureCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length) {
    HRESULT hr;

    if (!_composition) {
        hr = StartCompositionNow(ec);
        HRESULT_CHECK_RETURN(hr, L"StartCompositionNow failed");
    }

    return SetCompositionText(ec, str, length);
}

HRESULT Context::SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range) {
    HRESULT hr;

    if (_displayAtom == TF_INVALID_GUIDATOM) {
        return S_OK;
    }

    CComPtr<ITfProperty> prop;
    hr = _context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"context->GetProperty failed");

    CComVariant v = static_cast<int>(_displayAtom);
    hr = prop->SetValue(ec, range, &v);
    HRESULT_CHECK_RETURN(hr, L"prop->SetValue failed");

    return S_OK;
}

HRESULT Context::ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range) {
    HRESULT hr;

    CComPtr<ITfProperty> prop;
    hr = _context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"context->GetProperty failed");

    hr = prop->Clear(ec, range);
    HRESULT_CHECK_RETURN(hr, L"prop->Clear failed");

    return S_OK;
}

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

HRESULT Context::CommitCompositionText(_In_ TfEditCookie ec) {
    if (!GetComposition() || !GetEngine()->Count()) {
        return S_FALSE;
    }

    auto txstate = GetEngine()->Commit();
    if (txstate != Telex::TelexStates::Committed && txstate != Telex::TelexStates::CommittedInvalid) {
        assert(false);
        return E_FAIL;
    }

    auto str = GetEngine()->Retrieve();
    HRESULT hr = SetCompositionText(ec, str.c_str(), static_cast<LONG>(str.length()));
    DBG_HRESULT_CHECK(hr, L"SetCompositionText failed");

    // push the committed text out of the composition range
    CComPtr<ITfRange> compRange;
    if (SUCCEEDED(_composition->GetRange(&compRange))) {
        CComPtr<ITfRange> newStart;
        if (SUCCEEDED(compRange->Clone(&newStart)) && SUCCEEDED(newStart->Collapse(ec, TF_ANCHOR_END))) {
            hr = _composition->ShiftStart(ec, newStart);
            DBG_HRESULT_CHECK(hr, L"ShiftStart failed");

            TF_SELECTION sel = {newStart, {TF_AE_NONE, FALSE}};
            hr = _context->SetSelection(ec, 1, &sel);
            DBG_HRESULT_CHECK(hr, L"SetSelection failed");
        }
    }

    return S_OK;
}

HRESULT Context::EditCommit(_In_ TfEditCookie ec, _In_ wchar_t appendChar) {
    CommitCompositionText(ec);

    GetEngine()->Reset();
    EndCompositionNow(ec);

    if (appendChar != L'\0') {
        DBG_DPRINT(L"InsertTextAtSelection char=%lc", appendChar);

        CComPtr<ITfInsertAtSelection> insertAtSel;
        if (SUCCEEDED(_context->QueryInterface(&insertAtSel))) {
            CComPtr<ITfRange> insertedRange;
            HRESULT hr = insertAtSel->InsertTextAtSelection(ec, 0, &appendChar, 1, &insertedRange);
            DBG_HRESULT_CHECK(hr, L"InsertTextAtSelection failed");
        }
    }

    return S_OK;
}

} // namespace VietType
