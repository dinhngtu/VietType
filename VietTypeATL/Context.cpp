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

    if (pComposition) {
        EndCompositionNow(ecWrite, pComposition);
    }

    _engine->Reset();
    return S_OK;
}

STDMETHODIMP Context::OnEndEdit(
    __RPC__in_opt ITfContext* pic, TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord* pEditRecord) {
    HRESULT hr;

    if (pic != GetContext() || !IsCuas() || !pEditRecord) {
        return S_OK;
    }

    CComPtr<ITfComposition> composition;
    hr = GetComposition(ecReadOnly, &composition);
    if (FAILED(hr) || !composition) {
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
    CComPtr<ITfRange> selRange;
    selRange.Attach(sel.range);

    CComPtr<ITfRange> compRange;
    hr = composition->GetRange(&compRange);
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
        RequestEditKey(&hrSession, true, KeyResult::NotEatenEndComposition, L'\0');
    }

    return S_OK;
}

HRESULT Context::Initialize(
    _In_ ContextManager* parent,
    _In_ ITfContext* context,
    _In_ const Telex::TelexConfig& config,
    _In_ uint64_t configVersion,
    _In_ TfGuidAtom displayAttrAtom) {
    HRESULT hr;

    if (!context) {
        return E_INVALIDARG;
    }

    _parent = parent;
    _context = context;
    _displayAtom = displayAttrAtom;

    _engine.reset(Telex::TelexNew(config));
    _configVersion = configVersion;

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

HRESULT Context::GetComposition(_In_ TfEditCookie ec, _COM_Outptr_result_maybenull_ ITfComposition** composition) {
    HRESULT hr;

    *composition = nullptr;

    CComPtr<ITfContextComposition> contextComposition;
    hr = _context->QueryInterface(&contextComposition);
    HRESULT_CHECK_RETURN(hr, L"_context->QueryInterface failed");

    CComPtr<IEnumITfCompositionView> compositions;
    hr = contextComposition->FindComposition(ec, nullptr, &compositions);
    HRESULT_CHECK_RETURN(hr, L"contextComposition->FindComposition failed");

    while (1) {
        CComPtr<ITfCompositionView> compositionView;
        ULONG fetched;

        hr = compositions->Next(1, &compositionView, &fetched);
        if (FAILED(hr)) {
            return hr;
        }
        if (!fetched) {
            return S_FALSE;
        }

        CLSID clsid;
        if (FAILED(compositionView->GetOwnerClsid(&clsid)) || clsid != Globals::CLSID_TextService) {
            continue;
        }

        hr = compositionView->QueryInterface(composition);
        if (FAILED(hr)) {
            continue;
        }
    }
}

HRESULT Context::StartComposition() {
    HRESULT hr, hrSession;

    if (GetClientId() == TF_CLIENTID_NULL) {
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

    HRESULT hr, hrSession;

    hr = RequestEditSessionEx(&_EndComposition, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
    HRESULT_CHECK_RETURN(hr, "RequestEditSessionEx failed");
    HRESULT_CHECK_RETURN(hrSession, "_EndComposition failed");

    return S_OK;
}

} // namespace VietType
