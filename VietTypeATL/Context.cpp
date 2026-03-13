// SPDX-FileCopyrightText: Copyright (c) 2026 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "Telex.h"
#include "ContextManager.h"
#include "Compartment.h"

namespace VietType {

STDMETHODIMP Context::OnCompositionTerminated(_In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) {
    DBG_DPRINT(L"ecWrite = %ld", ecWrite);

    if (_composition == pComposition) {
        _engine->Reset();

        CComPtr<ITfRange> range;
        if (SUCCEEDED(_composition->GetRange(&range)) && _displayAtom != TF_INVALID_GUIDATOM) {
            ClearRangeDisplayAttribute(ecWrite, range);
        }

        // composition already dead
        _composition.Release();
    }

    return S_OK;
}

STDMETHODIMP Context::OnEndEdit(
    __RPC__in_opt ITfContext* pic, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord* pEditRecord) {
    HRESULT hr;

    if (!IsCuas() || !_composition || !pEditRecord) {
        return S_OK;
    }

    BOOL selChanged = FALSE;
    pEditRecord->GetSelectionStatus(&selChanged);
    if (!selChanged) {
        return S_OK;
    }

    TF_SELECTION sel;
    ULONG fetched = 0;
    hr = _context->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &sel, &fetched);
    if (FAILED(hr) || fetched == 0) {
        return S_OK;
    }
    CComPtr<ITfRange> selRange = sel.range;

    CComPtr<ITfRange> compRange;
    hr = _composition->GetRange(&compRange);
    if (FAILED(hr)) {
        return S_OK;
    }

    LONG cmpStart = 0, cmpEnd = 0;
    bool startOk = false, endOk = false;
    if (SUCCEEDED(compRange->CompareStart(ecReadOnly, selRange, TF_ANCHOR_START, &cmpStart))) {
        startOk = cmpStart <= 0;
    }
    if (SUCCEEDED(compRange->CompareEnd(ecReadOnly, selRange, TF_ANCHOR_END, &cmpEnd))) {
        endOk = cmpEnd >= 0;
    }
    if (!startOk || !endOk) {
        // might have closed the composition somehow (like clicking away in file rename box)?
        DBG_DPRINT(L"selection outside composition, committing (startOk=%d endOk=%d)", startOk, endOk);
        HRESULT hrSession;
        RequestEditKey(&hrSession, 0, 0, nullptr, false);
    }

    return S_OK;
}

HRESULT Context::Initialize(
    _In_ ContextManager* parent,
    _In_ ITfContext* context,
    _In_ const Telex::TelexConfig& config,
    _In_ TfGuidAtom displayAttrAtom) {
    HRESULT hr;

    if (!context) {
        return E_INVALIDARG;
    }

    _parent = parent;
    _context = context;
    _displayAtom = displayAttrAtom;

    _engine.reset(Telex::TelexNew(config));

    hr = _textEditSinkAdvisor.Advise(_context, this);
    DBG_HRESULT_CHECK(hr, L"_textEditSinkAdvisor.Advise failed");

    do {
        _isTransitory = false;

        TF_STATUS st;
        hr = _context->GetStatus(&st);
        if (FAILED(hr)) {
            break;
        }

        _isTransitory = SUCCEEDED(hr) && (st.dwStaticFlags & TF_SS_TRANSITORY);

        DBG_DPRINT(
            L"status d=%c%c%c%c%c%c%c%c%c s=%c%c%c%c%c%c%c",
            (st.dwDynamicFlags & TS_SD_READONLY) ? L'R' : L'_',
            (st.dwDynamicFlags & TS_SD_LOADING) ? L'L' : L'_',
            (st.dwDynamicFlags & TS_SD_RESERVED) ? L'?' : L'_',
            (st.dwDynamicFlags & TS_SD_TKBAUTOCORRECTENABLE) ? L'A' : L'_',
            (st.dwDynamicFlags & TS_SD_TKBPREDICTIONENABLE) ? L'P' : L'_',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'I' : L'_',
            (st.dwDynamicFlags & TS_SD_INPUTPANEMANUALDISPLAYENABLE) ? L'M' : L'_',
            (st.dwDynamicFlags & TS_SD_EMBEDDEDHANDWRITINGVIEW_ENABLED) ? L'H' : L'_',
            (st.dwDynamicFlags & TS_SD_EMBEDDEDHANDWRITINGVIEW_VISIBLE) ? L'V' : L'_',
            (st.dwStaticFlags & TS_SS_DISJOINTSEL) ? L'D' : L'_',
            (st.dwStaticFlags & TS_SS_REGIONS) ? L'R' : L'_',
            (st.dwStaticFlags & TS_SS_TRANSITORY) ? L'T' : L'_',
            (st.dwStaticFlags & TS_SS_NOHIDDENTEXT) ? L'H' : L'_',
            (st.dwStaticFlags & TS_SS_TKBAUTOCORRECTENABLE) ? L'A' : L'_',
            (st.dwStaticFlags & TS_SS_TKBPREDICTIONENABLE) ? L'P' : L'_',
            (st.dwStaticFlags & TS_SS_UWPCONTROL) ? L'U' : L'_');
    } while (0);

    do {
        _isCuas = false;

        CComPtr<ITfDocumentMgr> dim;
        hr = _context->GetDocumentMgr(&dim);
        if (FAILED(hr) || !dim) {
            break;
        }

        Compartment<long> emulated;
        hr = emulated.Initialize(dim, GetClientId(), Globals::GUID_Compartment_TsfEmulatedDocumentMgr);
        if (FAILED(hr)) {
            break;
        }

        long emulatedVal;
        hr = emulated.GetValue(&emulatedVal);
        if (FAILED(hr)) {
            break;
        }

        _isCuas = !!(emulatedVal & 1);
    } while (0);
    DBG_DPRINT(L"isCuas = %d", _isCuas);

    return S_OK;
}

HRESULT Context::Uninitialize() {
    _composition.Release();
    _engine.release();
    _blocked = true;
    _displayAtom = TF_INVALID_GUIDATOM;
    _textEditSinkAdvisor.Unadvise();
    _isTransitory = false;
    _isCuas = false;
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
