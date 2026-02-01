// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "CompositionManager.h"
#include "EngineController.h"
#include "EngineSettingsController.h"
#include "EditSessions.h"

namespace VietType {

static HRESULT IsContextEmpty(_In_ ITfContext* context, _In_ TfClientId clientid, _Out_ bool* isempty) {
    HRESULT hr;

    Compartment<long> compEmpty;
    hr = compEmpty.Initialize(context, clientid, GUID_COMPARTMENT_EMPTYCONTEXT);
    HRESULT_CHECK_RETURN(hr, L"compEmpty->Initialize failed");

    long contextEmpty;
    hr = compEmpty.GetValue(&contextEmpty);
    HRESULT_CHECK_RETURN(hr, L"compDisabled->GetValue failed");

    *isempty = hr == S_OK && contextEmpty;
    return hr;
}

HRESULT CompositionManager::OnNewContext(_In_opt_ ITfContext* context) {
    HRESULT hr;

    _context = context;

    if (!context) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    bool isempty;
    hr = IsContextEmpty(context, _clientid, &isempty);
    HRESULT_CHECK_RETURN(hr, L"IsContextEmpty failed");
    if (isempty) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

#ifdef _DEBUG
    TF_STATUS st;
    hr = context->GetStatus(&st);
    DBG_HRESULT_CHECK(hr, L"context->GetStatus failed") else {
        DBG_DPRINT(
            L"d=%c%c%c%c%c%c%c%c%c s=%c%c%c%c%c%c%c",
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
    }
#endif

    Compartment<long> compBackconvert;
    hr = compBackconvert.Initialize(context, _clientid, Globals::GUID_Compartment_Backconvert);
    if (SUCCEEDED(hr)) {
        compBackconvert.SetValue(0);
    } else {
        DBG_HRESULT_CHECK(hr, L"compBackconvert.Initialize failed");
    }

    hr = RequestEditSession(EditSessions::EditBlocked, _controller.p);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"CompositionManager::RequestEditSession failed");
        _controller->SetBlocked(EngineController::BlockedKind::Free);
    }

    return S_OK;
}

STDMETHODIMP CompositionManager::OnInitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    DBG_DPRINT(L"pdim = %p", pdim);

    return E_NOTIMPL;
}

STDMETHODIMP CompositionManager::OnUninitDocumentMgr(__RPC__in_opt ITfDocumentMgr* pdim) {
    DBG_DPRINT(L"pdim = %p", pdim);

    return E_NOTIMPL;
}

STDMETHODIMP CompositionManager::OnSetFocus(
    __RPC__in_opt ITfDocumentMgr* pdimFocus, __RPC__in_opt ITfDocumentMgr* pdimPrevFocus) {
    HRESULT hr;

    DBG_DPRINT(L"pdimFocus = %p", pdimFocus);

    _composition.Release();
    _context.Release();

    if (!pdimFocus) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    CComPtr<ITfContext> context;
    hr = pdimFocus->GetTop(&context);
    if (FAILED(hr)) {
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    hr = OnNewContext(context);
    HRESULT_CHECK_RETURN(hr, L"OnNewContext failed");

    return S_OK;
}

STDMETHODIMP CompositionManager::OnPushContext(__RPC__in_opt ITfContext* pic) {
    DBG_DPRINT(L"context = %p", pic);

    _composition.Release();

    return S_OK;
}

STDMETHODIMP CompositionManager::OnPopContext(__RPC__in_opt ITfContext* pic) {
    DBG_DPRINT(L"context = %p", pic);

    _composition.Release();

    return S_OK;
}

STDMETHODIMP CompositionManager::OnCompositionTerminated(
    _In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) {
    HRESULT hr;

    DBG_DPRINT(L"ecWrite = %ld", ecWrite);

    if (_composition == pComposition) {
        _controller->GetEngine().Reset();
        hr = EndCompositionNow(ecWrite);
        DBG_HRESULT_CHECK(hr, L"EndCompositionNow failed");
    }

    return S_OK;
}

_Check_return_ HRESULT CompositionManager::Initialize(
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId clientid,
    _In_ EngineController* controller,
    _In_opt_ ITfDisplayAttributeInfo* composingAttribute,
    _In_ bool comless) {
    HRESULT hr;

    _clientid = clientid;
    _controller = controller;
    _composingAttribute = composingAttribute;
    _threadMgr = threadMgr;

    hr = _threadMgrEventSinkAdvisor.Advise(threadMgr, this);
    HRESULT_CHECK_RETURN(hr, L"_threadMgrEventSinkAdvisor.Advise failed");

    CComPtr<ITfKeystrokeMgr> keystrokeMgr;
    hr = _threadMgr->QueryInterface(&keystrokeMgr);
    if (SUCCEEDED(hr)) {
        hr = keystrokeMgr->AdviseKeyEventSink(_clientid, this, TRUE);
        HRESULT_CHECK_RETURN(hr, L"_keystrokeMgr->AdviseKeyEventSink failed");

        _controller->GetSettings()->GetPreservedKeyToggle(&_pk_toggle);
        hr = keystrokeMgr->PreserveKey(_clientid, GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle, NULL, 0);
        // probably not fatal
        DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->PreserveKey failed");
    } else {
        DBG_HRESULT_CHECK(hr, L"_threadMgr->QueryInterface failed");
    }

    if (!comless) {
        hr = _categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
        HRESULT_CHECK_RETURN(hr, L"_categoryMgr.CoCreateInstance failed");
    }

    return S_OK;
}

HRESULT CompositionManager::Uninitialize() {
    HRESULT hr;

    _composition.Release();
    _context.Release();

    _categoryMgr.Release();

    CComPtr<ITfKeystrokeMgr> keystrokeMgr;
    hr = _threadMgr->QueryInterface(&keystrokeMgr);
    if (SUCCEEDED(hr)) {
        hr = keystrokeMgr->UnpreserveKey(GUID_KeyEventSink_PreservedKey_Toggle, &_pk_toggle);
        DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->UnpreserveKey failed");

        hr = keystrokeMgr->UnadviseKeyEventSink(_clientid);
        DBG_HRESULT_CHECK(hr, L"_keystrokeMgr->UnadviseKeyEventSink failed");
    } else {
        DBG_HRESULT_CHECK(hr, L"_threadMgr->QueryInterface failed");
    }

    _threadMgrEventSinkAdvisor.Unadvise();
    _threadMgr.Release();

    _composingAttribute.Release();
    _controller.Release();
    _clientid = TF_CLIENTID_NULL;

    return S_OK;
}

HRESULT CompositionManager::StartComposition(_In_ ITfContext* pContext) {
    if (_clientid == TF_CLIENTID_NULL || _composition) {
        DBG_DPRINT(L"bad composition request");
        return E_FAIL;
    }
    return RequestEditSession(&_StartComposition);
}

HRESULT CompositionManager::EndComposition() {
    if (_clientid == TF_CLIENTID_NULL) {
        DBG_DPRINT(L"bad end composition request");
        return E_FAIL;
    }
    if (_composition) {
        return RequestEditSession(&_EndComposition);
    } else {
        return S_OK;
    }
}

bool CompositionManager::IsComposing() const {
    return (bool)_composition;
}

_Ret_maybenull_ ITfComposition* CompositionManager::GetComposition() const {
    return _composition;
}

_Check_return_ HRESULT CompositionManager::GetRange(_COM_Outptr_ ITfRange** range) {
    if (!IsComposing()) {
        *range = nullptr;
        return E_FAIL;
    }
    HRESULT hr = _composition->GetRange(range);
    HRESULT_CHECK_RETURN_OUTPTR(hr, range, L"_composition->GetRange failed");

    return S_OK;
}

TfClientId CompositionManager::GetClientId() const {
    return _clientid;
}

HRESULT CompositionManager::StartCompositionNow(_In_ TfEditCookie ec, _In_ ITfContext* context) {
    HRESULT hr;

    if (_composition) {
        return TF_E_LOCKED;
    }

    CComPtr<ITfCompositionSink> compositionSink;
    hr = QueryInterface2(this, &compositionSink);
    HRESULT_CHECK_RETURN(hr, L"this->QueryInterface failed");

    CComPtr<ITfInsertAtSelection> insertAtSelection;
    hr = context->QueryInterface(&insertAtSelection);
    HRESULT_CHECK_RETURN(hr, L"context->QueryInterface failed");

    CComPtr<ITfRange> insertRange;
    hr = insertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &insertRange);
    HRESULT_CHECK_RETURN(hr, L"insertAtSelection->InsertTextAtSelection failed");

    CComPtr<ITfContextComposition> contextComposition;
    hr = context->QueryInterface(&contextComposition);
    HRESULT_CHECK_RETURN(hr, L"context->QueryInterface failed");

    ITfComposition* composition;
    hr = contextComposition->StartComposition(ec, insertRange, compositionSink, &composition);
    HRESULT_CHECK_RETURN(hr, L"contextComposition->StartComposition failed") else {
        _composition = composition;

        TF_SELECTION sel;
        sel.range = insertRange;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = context->SetSelection(ec, 1, &sel);
        DBG_HRESULT_CHECK(hr, L"context->SetSelection failed");

        _context = context;
    }

    return S_OK;
}

HRESULT CompositionManager::EmptyCompositionText(_In_ TfEditCookie ec) {
    HRESULT hr;

    CComPtr<ITfRange> range;
    hr = _composition->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"composition->GetRange failed");

    hr = range->SetText(ec, 0, NULL, 0);
    HRESULT_CHECK_RETURN(hr, L"range->SetText failed");

    return S_OK;
}

HRESULT CompositionManager::MoveCaretToEnd(_In_ TfEditCookie ec) {
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

HRESULT CompositionManager::EndCompositionNow(_In_ TfEditCookie ec) {
    HRESULT hr;

    DBG_DPRINT(L"ending composition");

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        if (SUCCEEDED(hr)) {
            if (_composingAttribute) {
                hr = ClearRangeDisplayAttribute(ec, _context, range);
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

HRESULT CompositionManager::SetCompositionText(_In_ TfEditCookie ec, _In_z_ LPCWSTR str, _In_ LONG length) {
    HRESULT hr;

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"_composition->GetRange failed");

        hr = range->SetText(ec, TF_ST_CORRECTION, str, length);
        HRESULT_CHECK_RETURN(hr, L"range->SetText failed");

        hr = SetRangeDisplayAttribute(ec, _context, range);
        HRESULT_CHECK_RETURN(hr, L"SetRangeDisplayAttribute failed");

        hr = MoveCaretToEnd(ec);
        DBG_HRESULT_CHECK(hr, L"MoveCaretToEnd failed");
    }

    return S_OK;
}

HRESULT CompositionManager::EnsureCompositionText(
    _In_ TfEditCookie ec, _In_ ITfContext* context, _In_z_ LPCWSTR str, _In_ LONG length) {
    HRESULT hr;

    if (!_composition) {
        hr = StartComposition(context);
        HRESULT_CHECK_RETURN(hr, L"StartComposition failed");
    }

    return SetCompositionText(ec, str, length);
}

HRESULT CompositionManager::SetRangeDisplayAttribute(
    _In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range) {
    HRESULT hr;

    if (!_categoryMgr || !_composingAttribute) {
        return S_OK;
    }

    GUID guid;
    hr = _composingAttribute->GetGUID(&guid);
    HRESULT_CHECK_RETURN(hr, L"attr->GetGUID failed");

    TfGuidAtom atom = TF_INVALID_GUIDATOM;
    hr = _categoryMgr->RegisterGUID(guid, &atom);
    HRESULT_CHECK_RETURN(hr, L"_categoryMgr->RegisterGUID failed");

    if (atom == TF_INVALID_GUIDATOM) {
        return E_FAIL;
    }

    CComPtr<ITfProperty> prop;
    hr = context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"context->GetProperty failed");

    CComVariant v = static_cast<int>(atom);
    hr = prop->SetValue(ec, range, &v);
    HRESULT_CHECK_RETURN(hr, L"prop->SetValue failed");

    return S_OK;
}

HRESULT CompositionManager::ClearRangeDisplayAttribute(
    _In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range) {
    HRESULT hr;

    CComPtr<ITfProperty> prop;
    hr = context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"context->GetProperty failed");

    hr = prop->Clear(ec, range);
    HRESULT_CHECK_RETURN(hr, L"prop->Clear failed");

    return S_OK;
}

HRESULT CompositionManager::_StartComposition(
    _In_ TfEditCookie ec, _In_ CompositionManager* instance, _In_ ITfContext* context) {
    return instance->StartCompositionNow(ec, context);
}

HRESULT CompositionManager::_EndComposition(
    _In_ TfEditCookie ec, _In_ CompositionManager* instance, _In_ ITfContext* context) {
    return instance->EndCompositionNow(ec);
}

} // namespace VietType
