// SPDX-FileCopyrightText: Copyright (c) 2025 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "KeyTranslator.h"

namespace VietType {

static PCWSTR GetTelexStateName(Telex::TelexStates state) {
    switch (state) {
    case Telex::TelexStates::Valid:
        return L"Valid";
    case Telex::TelexStates::Invalid:
        return L"Invalid";
    case Telex::TelexStates::Committed:
        return L"Committed";
    case Telex::TelexStates::CommittedInvalid:
        return L"CommittedInvalid";
    case Telex::TelexStates::BackconvertFailed:
        return L"BackconvertFailed";
    case Telex::TelexStates::TxError:
        return L"TxError";
    default:
        return L"Unknown";
    }
}

HRESULT Context::StartCompositionNow(_In_ TfEditCookie ec, _COM_Outptr_opt_ ITfComposition** outComposition) {
    HRESULT hr;

    if (outComposition) {
        *outComposition = nullptr;
    }

    CComPtr<ITfCompositionSink> compositionSink;
    hr = QueryInterface2(this, &compositionSink);
    HRESULT_CHECK_RETURN(hr, L"this->QueryInterface failed");

    CComPtr<ITfInsertAtSelection> insertAtSelection;
    hr = _context->QueryInterface(&insertAtSelection);
    HRESULT_CHECK_RETURN(hr, L"_context->QueryInterface failed");

    CComPtr<ITfRange> insertRange;
    hr = insertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &insertRange);
    HRESULT_CHECK_RETURN(hr, L"insertAtSelection->InsertTextAtSelection failed");

    CComPtr<ITfContextComposition> contextComposition;
    hr = _context->QueryInterface(&contextComposition);
    HRESULT_CHECK_RETURN(hr, L"_context->QueryInterface failed");

    CComPtr<ITfComposition> composition;
    hr = contextComposition->StartComposition(ec, insertRange, compositionSink, &composition);
    HRESULT_CHECK_RETURN(hr, L"contextComposition->StartComposition failed");

    if (!composition) {
        return E_ABORT;
    }

    DBG_DPRINT(L"started composition %p", composition.p);

    if (outComposition) {
        *outComposition = composition;
        (*outComposition)->AddRef();
    }
    return S_OK;
}

HRESULT Context::EndCompositionNow(_In_ TfEditCookie ec, _In_opt_ ITfComposition* composition) {
    HRESULT hr;

    if (!composition) {
        return S_OK;
    }

    DBG_DPRINT(L"ending composition %p", composition);

    CComPtr<ITfRange> range;
    hr = composition->GetRange(&range);
    HRESULT_CHECK(hr, L"composition->GetRange failed");
    if (SUCCEEDED(hr)) {
        if (_displayAtom != TF_INVALID_GUIDATOM) {
            hr = ClearRangeDisplayAttribute(ec, range);
            DBG_HRESULT_CHECK(hr, L"ClearRangeDisplayAttribute failed");
        }

        hr = MoveCaretToEnd(ec, composition, true);
        DBG_HRESULT_CHECK(hr, L"MoveCaretToEnd failed");

        hr = composition->EndComposition(ec);
        DBG_HRESULT_CHECK(hr, L"composition->EndComposition failed");
    }

    return S_OK;
}

HRESULT Context::EnsureComposition(_In_ TfEditCookie ec, CComPtr<ITfComposition>& composition) {
    if (composition) {
        return S_OK;
    }
    HRESULT hr = GetComposition(ec, &composition);
    if (SUCCEEDED(hr) && composition) {
        DBG_DPRINT(L"existing composition %p", composition.p);
        return S_OK;
    }
    return StartCompositionNow(ec, &composition);
}

HRESULT Context::SetCompositionText(
    _In_ TfEditCookie ec, _In_ ITfComposition* composition, _In_z_ LPCWSTR str, _In_ LONG length) {
    HRESULT hr;

    CComPtr<ITfRange> range;
    hr = composition->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"composition->GetRange failed");

    hr = range->SetText(ec, 0, str, length);
    HRESULT_CHECK_RETURN(hr, L"range->SetText failed");

    hr = SetRangeDisplayAttribute(ec, range);
    HRESULT_CHECK_RETURN(hr, L"SetRangeDisplayAttribute failed");

    hr = MoveCaretToEnd(ec, composition);
    DBG_HRESULT_CHECK(hr, L"MoveCaretToEnd failed");

    return S_OK;
}

HRESULT Context::EmptyCompositionText(_In_ TfEditCookie ec, _In_ ITfComposition* composition) {
    HRESULT hr;

    CComPtr<ITfRange> range;
    hr = composition->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"composition->GetRange failed");

    hr = range->SetText(ec, 0, nullptr, 0);
    HRESULT_CHECK_RETURN(hr, L"range->SetText failed");

    return S_OK;
}

HRESULT Context::MoveCaretToEnd(_In_ TfEditCookie ec, _In_ ITfComposition* composition, bool collapse) {
    HRESULT hr;

    CComPtr<ITfRange> range;
    hr = composition->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"composition->GetRange failed");

    CComPtr<ITfRange> rangeClone;
    hr = range->Clone(&rangeClone);
    HRESULT_CHECK_RETURN(hr, L"range->Clone failed");

    hr = rangeClone->Collapse(ec, TF_ANCHOR_END);
    HRESULT_CHECK_RETURN(hr, L"rangeClone->Collapse failed");

    if (collapse) {
        hr = composition->ShiftStart(ec, rangeClone);
        HRESULT_CHECK_RETURN(hr, L"composition->ShiftStart failed");
    }

    TF_SELECTION sel;
    sel.range = rangeClone;
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = FALSE;
    hr = _context->SetSelection(ec, 1, &sel);
    HRESULT_CHECK_RETURN(hr, L"_context->SetSelection failed");

    return S_OK;
}

HRESULT Context::SetRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range) {
    HRESULT hr;

    if (_displayAtom == TF_INVALID_GUIDATOM) {
        return S_OK;
    }

    CComPtr<ITfProperty> prop;
    hr = _context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"_context->GetProperty failed");

    CComVariant v = static_cast<int>(_displayAtom);
    hr = prop->SetValue(ec, range, &v);
    HRESULT_CHECK_RETURN(hr, L"prop->SetValue failed");

    return S_OK;
}

HRESULT Context::ClearRangeDisplayAttribute(_In_ TfEditCookie ec, _In_ ITfRange* range) {
    HRESULT hr;

    CComPtr<ITfProperty> prop;
    hr = _context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"_context->GetProperty failed");

    hr = prop->Clear(ec, range);
    HRESULT_CHECK_RETURN(hr, L"prop->Clear failed");

    return S_OK;
}

HRESULT Context::DoEditNextState(
    _In_ TfEditCookie ec,
    _In_ Telex::TelexStates state,
    _In_ wchar_t nonEngineAppend,
    _In_opt_ ITfComposition* existingComposition,
    _In_ bool newComposition,
    _In_ bool resetAnyway) {
    HRESULT hr;

    std::wstring str;
    auto prevCount = GetEngine()->Count();
    switch (state) {
    case Telex::TelexStates::Valid:
        str = GetEngine()->Peek();
        break;
    case Telex::TelexStates::Invalid:
        str = GetEngine()->RetrieveRaw();
        break;
    case Telex::TelexStates::Committed:
        str = GetEngine()->Retrieve();
        GetEngine()->Reset();
        break;
    default:
        str = GetEngine()->RetrieveRaw();
        GetEngine()->Reset();
        break;
    }
    if (resetAnyway) {
        GetEngine()->Reset();
    }

    if (nonEngineAppend) {
        str += nonEngineAppend;
    }

    CComPtr<ITfComposition> composition(existingComposition);
    if (newComposition) {
        hr = EndCompositionNow(ec, composition);
        HRESULT_CHECK(hr, L"EndCompositionNow failed");
        composition.Release();
    } else {
        if (!composition) {
            hr = GetComposition(ec, &composition);
            HRESULT_CHECK(hr, L"context->GetComposition failed");
        }
    }

    DBG_DPRINT(
        L"existing %p, current %p, %s composition, %s, "
        L"state %s, count %zu, append '%c', str '%s', next state %s, next count %zu",
        existingComposition,
        composition.p,
        newComposition ? L"new" : L"existing",
        resetAnyway ? L"reset" : L"no reset",
        GetTelexStateName(state),
        prevCount,
        nonEngineAppend ? nonEngineAppend : L'_',
        str.c_str(),
        GetTelexStateName(GetEngine()->GetState()),
        GetEngine()->Count());

    // sync the composition state with the supposed string state
    if (!str.empty()) {
        if (!composition) {
            hr = StartCompositionNow(ec, &composition);
            HRESULT_CHECK_RETURN(hr, L"StartCompositionNow failed");
        }
        hr = SetCompositionText(ec, composition, &str[0], static_cast<LONG>(str.length()));
        HRESULT_CHECK_RETURN(hr, L"SetCompositionText failed");
    } else if (composition) {
        // EndComposition* will not empty composition text so we have to do it manually
        hr = EmptyCompositionText(ec, composition);
        HRESULT_CHECK_RETURN(hr, L"EmptyCompositionText failed");
    }

    // now sync the composition state with the engine state
    if (GetEngine()->Count()) {
        // EnsureComposition should already be called above
    } else if (composition) {
        hr = EndCompositionNow(ec, composition);
        HRESULT_CHECK_RETURN(hr, L"EndCompositionNow failed");
        composition.Release();
    }

    return S_OK;
}

HRESULT Context::EditKey(
    _In_ TfEditCookie ec,
    _In_ Context* context,
    _In_ KeyResult keyResult,
    _In_ wchar_t push,
    _In_ bool newComposition) {
    DBG_DPRINT(L"ec = %ld %s '%c'", ec, GetKeyResult(keyResult), push ? push : L'_');

    HRESULT hr;
    Telex::ITelexEngine* engine = context->GetEngine();

    CComPtr<ITfComposition> composition;
    hr = context->GetComposition(ec, &composition);
    HRESULT_CHECK_RETURN(hr, L"context->GetComposition failed");

    switch (keyResult) {
    case KeyResult::BreakingCharacter:
        return context->DoEditNextState(ec, engine->Commit(), push, composition, newComposition);
    case KeyResult::Character:
        return context->DoEditNextState(ec, engine->PushChar(push), L'\0', composition, newComposition);
    case KeyResult::Backspace:
        return context->DoEditNextState(ec, engine->Backspace(), L'\0', composition, newComposition);
    case KeyResult::Escape:
        return context->DoEditNextState(ec, engine->Cancel(), L'\0', composition, newComposition);
    case KeyResult::NotEaten:
    case KeyResult::NotEatenEndComposition:
    default:
        // return context->DoEditNextState(ec, engine->GetState(), L'\0', composition, newComposition, true);
        engine->Reset();
        return context->EndCompositionNow(ec, composition);
    }
}

} // namespace VietType
