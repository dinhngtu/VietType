// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// Derived from Microsoft's SampleIME source code included in the Windows classic samples,
// whose original copyright and permission notice is included below:
//
//     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//     ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//     THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//     PARTICULAR PURPOSE.
//
//     Copyright (c) Microsoft Corporation. All rights reserved

#include "CompositionManager.h"

namespace VietType {

STDMETHODIMP CompositionManager::OnCompositionTerminated(
    _In_ TfEditCookie ecWrite, __RPC__in_opt ITfComposition* pComposition) {
    HRESULT hr;

    DBG_DPRINT(L"ecWrite = %ld", ecWrite);

    hr = EndCompositionNow(ecWrite);
    DBG_HRESULT_CHECK(hr, L"EndCompositionNow failed");

    return S_OK;
}

_Check_return_ HRESULT CompositionManager::Initialize(
    _In_ TfClientId clientid, _In_opt_ ITfDisplayAttributeInfo* composingAttribute, _In_ bool comless) {
    HRESULT hr;

    _clientid = clientid;
    _composingAttribute = composingAttribute;

    if (!comless) {
        hr = _categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
        HRESULT_CHECK_RETURN(hr, L"_categoryMgr.CoCreateInstance failed");
    }

    return S_OK;
}

HRESULT CompositionManager::Uninitialize() {
    _categoryMgr.Release();
    _composingAttribute.Release();
    return S_OK;
}

HRESULT CompositionManager::RequestEditSession(
    _In_ ITfEditSession* session, _In_ ITfContext* context, _In_ DWORD flags) {
    if (_clientid == TF_CLIENTID_NULL || !context) {
        DBG_DPRINT(L"bad edit session request");
        return E_FAIL;
    }
    HRESULT hrSession;
    return context->RequestEditSession(_clientid, session, flags, &hrSession);
}

HRESULT CompositionManager::StartComposition(_In_ ITfContext* pContext) {
    if (_clientid == TF_CLIENTID_NULL || _composition) {
        DBG_DPRINT(L"bad composition request");
        return E_FAIL;
    }
    return RequestEditSession(&_StartComposition, this, pContext);
}

HRESULT CompositionManager::EndComposition() {
    if (_clientid == TF_CLIENTID_NULL) {
        DBG_DPRINT(L"bad end composition request");
        return E_FAIL;
    }
    if (_composition) {
        return RequestEditSession(&_EndComposition, this, _context);
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
        HRESULT_CHECK_RETURN(hr, L"_composition->GetRange failed");

        if (_composingAttribute) {
            hr = ClearRangeDisplayAttribute(ec, _context, range);
            HRESULT_CHECK_RETURN(hr, L"ClearRangeDisplayAttribute failed");
        }

        hr = MoveCaretToEnd(ec);
        DBG_HRESULT_CHECK(hr, L"MoveCaretToEnd failed");

        hr = _composition->EndComposition(ec);
        DBG_HRESULT_CHECK(hr, L"_composition->EndComposition failed");

        _composition.Release();
        _context.Release();
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

        if (_composingAttribute) {
            hr = SetRangeDisplayAttribute(ec, _context, range, _composingAttribute);
            HRESULT_CHECK_RETURN(hr, L"SetRangeDisplayAttribute failed");
        }

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
    _In_ TfEditCookie ec, _In_ ITfContext* context, _In_ ITfRange* range, _In_ ITfDisplayAttributeInfo* attr) {
    HRESULT hr;

    if (!_categoryMgr) {
        return S_OK;
    }

    GUID guid;
    hr = attr->GetGUID(&guid);
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
