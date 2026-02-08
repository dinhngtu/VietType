// SPDX-FileCopyrightText: Copyright (c) 2026 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "Telex.h"
#include "ContextManager.h"

namespace VietType {

STDMETHODIMP Context::OnCompositionTerminated(_In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) {
    HRESULT hr;

    DBG_DPRINT(L"ecWrite = %ld", ecWrite);

    if (_composition == pComposition) {
        _engine->Reset();
        hr = EndCompositionNow(ecWrite);
        DBG_HRESULT_CHECK(hr, L"EndCompositionNow failed");
    }

    return S_OK;
}

HRESULT Context::Initialize(
    _In_ ContextManager* parent,
    _In_ ITfContext* context,
    _In_ const Telex::TelexConfig& config,
    _In_ TfGuidAtom displayAttrAtom) {
    if (!context) {
        return E_INVALIDARG;
    }

    _parent = parent;
    _context = context;
    _displayAtom = displayAttrAtom;

    _engine.reset(Telex::TelexNew(config));

    return S_OK;
}

HRESULT Context::Uninitialize() {
    _composition.Release();
    _engine.release();
    _blocked = true;
    _displayAtom = TF_INVALID_GUIDATOM;
    _context.Release();
    _parent = nullptr;

    return S_OK;
}

TfClientId Context::GetClientId() const {
    return _parent->GetClientId();
}

void Context::UpdateStatus() {
    HRESULT hr = _parent->UpdateStatus(this);
    DBG_HRESULT_CHECK(hr, "_parent->UpdateStatus failed");
}

HRESULT Context::StartComposition() {
    HRESULT hr, hrSession;

    if (GetClientId() == TF_CLIENTID_NULL || _composition) {
        DBG_DPRINT(L"bad composition request");
        return E_FAIL;
    }

    hr = RequestEditSessionEx(&_StartComposition, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
    HRESULT_CHECK_RETURN(hr, "RequestEditSessionEx failed");
    HRESULT_CHECK_RETURN(hrSession, "_StartComposition failed");

    return S_OK;
}

HRESULT Context::EndComposition() {
    if (GetClientId() == TF_CLIENTID_NULL) {
        DBG_DPRINT(L"bad end composition request");
        return E_FAIL;
    }

    if (_composition) {
        HRESULT hr, hrSession;

        hr = RequestEditSessionEx(&_EndComposition, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        HRESULT_CHECK_RETURN(hr, "RequestEditSessionEx failed");
        HRESULT_CHECK_RETURN(hrSession, "_EndComposition failed");
    }

    return S_OK;
}

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
    HRESULT_CHECK_RETURN(hr, L"contextComposition->StartComposition failed") else {
        _composition = composition;

        TF_SELECTION sel;
        sel.range = insertRange;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = _context->SetSelection(ec, 1, &sel);
        DBG_HRESULT_CHECK(hr, L"context->SetSelection failed");
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

        hr = range->SetText(ec, TF_ST_CORRECTION, str, length);
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

} // namespace VietType
