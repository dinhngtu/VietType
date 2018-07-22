// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "CompositionManager.h"

VietType::CompositionManager::CompositionManager() {
}

VietType::CompositionManager::~CompositionManager() {
}


STDMETHODIMP VietType::CompositionManager::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition * pComposition) {
    HRESULT hr;

    DBG_DPRINT(L"ecWrite = %ld", ecWrite);

    hr = EndCompositionNow(ecWrite);
    DBG_HRESULT_CHECK(hr, L"%s", L"EndCompositionNow failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::Initialize(TfClientId clientid, CComPtr<ITfDisplayAttributeInfo> const& composingAttribute, bool comless) {
    HRESULT hr;

    _clientid = clientid;
    _composingAttribute = composingAttribute;

    if (!comless) {
        hr = _categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_categoryMgr.CoCreateInstance failed");
    }

    return S_OK;
}

void VietType::CompositionManager::Uninitialize() {
    _categoryMgr.Release();
    _composingAttribute.Release();
}


HRESULT VietType::CompositionManager::RequestEditSession(ITfEditSession * session) {
    return RequestEditSession(session, _context);
}

HRESULT VietType::CompositionManager::RequestEditSession(ITfEditSession *session, ITfContext *context) {
    assert(_clientid != TF_CLIENTID_NULL);
    assert(context);
    HRESULT hrSession;
    return context->RequestEditSession(_clientid, session, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
}

HRESULT VietType::CompositionManager::StartComposition(ITfContext * pContext) {
    assert(_clientid != TF_CLIENTID_NULL);
    assert(!_composition);
    return RequestEditSession(&_StartComposition, this, pContext);
}

HRESULT VietType::CompositionManager::EndComposition() {
    assert(_clientid != TF_CLIENTID_NULL);
    if (_composition) {
        return RequestEditSession(&_EndComposition, this, _context);
    } else {
        return S_OK;
    }
}

bool VietType::CompositionManager::IsComposing() const {
    return (bool)_composition;
}

CComPtr<ITfComposition> const & VietType::CompositionManager::GetComposition() const {
    return _composition;
}

HRESULT VietType::CompositionManager::GetRange(ITfRange ** range) {
    if (!IsComposing()) {
        return E_FAIL;
    }
    return _composition->GetRange(range);
}

TfClientId VietType::CompositionManager::GetClientId() const {
    return _clientid;
}

HRESULT VietType::CompositionManager::StartCompositionNow(TfEditCookie ec, ITfContext * context) {
    HRESULT hr;

    CComPtr<ITfCompositionSink> compositionSink;
    hr = QueryInterface2(this, &compositionSink);
    HRESULT_CHECK_RETURN(hr, L"%s", L"this->QueryInterface failed");

    CComPtr<ITfInsertAtSelection> insertAtSelection;
    hr = context->QueryInterface(&insertAtSelection);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->QueryInterface failed");

    CComPtr<ITfRange> insertRange;
    hr = insertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &insertRange);
    HRESULT_CHECK_RETURN(hr, L"%s", L"insertAtSelection->InsertTextAtSelection failed");

    CComPtr<ITfContextComposition> contextComposition;
    hr = context->QueryInterface(&contextComposition);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->QueryInterface failed");

    ITfComposition *composition;
    hr = contextComposition->StartComposition(ec, insertRange, compositionSink, &composition);
    if (SUCCEEDED(hr)) {
        _composition = composition;

        TF_SELECTION sel;
        sel.range = insertRange;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = context->SetSelection(ec, 1, &sel);
        DBG_HRESULT_CHECK(hr, L"%s", L"context->SetSelection failed");

        _context = context;
    } else HRESULT_CHECK_RETURN(hr, L"%s", L"contextComposition->StartComposition failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::EmptyCompositionText(TfEditCookie ec) {
    HRESULT hr;

    CComPtr<ITfRange> range;
    hr = _composition->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"%s", L"composition->GetRange failed");
    
    hr = range->SetText(ec, 0, NULL, 0);
    HRESULT_CHECK_RETURN(hr, L"%s", L"range->SetText failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::MoveCaretToEnd(TfEditCookie ec) {
    HRESULT hr;

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_composition->GetRange failed");

        CComPtr<ITfRange> rangeClone;
        hr = range->Clone(&rangeClone);
        HRESULT_CHECK_RETURN(hr, L"%s", L"range->Clone failed");

        hr = rangeClone->Collapse(ec, TF_ANCHOR_END);
        HRESULT_CHECK_RETURN(hr, L"%s", L"rangeClone->Collapse failed");

        TF_SELECTION sel;
        sel.range = rangeClone;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = _context->SetSelection(ec, 1, &sel);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_context->SetSelection failed");
    }

    return S_OK;
}

HRESULT VietType::CompositionManager::EndCompositionNow(TfEditCookie ec) {
    HRESULT hr;

    DBG_DPRINT(L"%s", L"ending composition");

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_composition->GetRange failed");

        hr = ClearRangeDisplayAttribute(ec, _context, range);
        HRESULT_CHECK_RETURN(hr, L"%s", L"ClearRangeDisplayAttribute failed");

        hr = MoveCaretToEnd(ec);
        DBG_HRESULT_CHECK(hr, L"%s", L"MoveCaretToEnd failed");

        hr = _composition->EndComposition(ec);
        DBG_HRESULT_CHECK(hr, L"%s", L"_composition->EndComposition failed");

        _composition.Release();
        _context.Release();
    }

    return S_OK;
}

HRESULT VietType::CompositionManager::SetCompositionText(TfEditCookie ec, WCHAR const * str, LONG length) {
    HRESULT hr;

    if (_composition) {
        CComPtr<ITfRange> range;
        hr = _composition->GetRange(&range);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_composition->GetRange failed");

        hr = range->SetText(ec, TF_ST_CORRECTION, str, length);
        HRESULT_CHECK_RETURN(hr, L"%s", L"range->SetText failed");

        hr = SetRangeDisplayAttribute(ec, _context, range, _composingAttribute);
        HRESULT_CHECK_RETURN(hr, L"%s", L"SetRangeDisplayAttribute failed");

        hr = MoveCaretToEnd(ec);
        DBG_HRESULT_CHECK(hr, L"%s", L"MoveCaretToEnd failed");
    }

    return S_OK;
}

HRESULT VietType::CompositionManager::EnsureCompositionText(TfEditCookie ec, ITfContext *context, WCHAR const * str, LONG length) {
    HRESULT hr;

    if (!_composition) {
        hr = StartComposition(context);
        HRESULT_CHECK_RETURN(hr, L"%s", L"StartComposition failed");
    }

    return SetCompositionText(ec, str, length);
}

HRESULT VietType::CompositionManager::SetRangeDisplayAttribute(TfEditCookie ec, ITfContext *context, ITfRange * range, ITfDisplayAttributeInfo * attr) {
    HRESULT hr;

    if (!_categoryMgr) {
        return S_OK;
    }

    GUID guid;
    hr = attr->GetGUID(&guid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"attr->GetGUID failed");

    TfGuidAtom atom = TF_INVALID_GUIDATOM;
    hr = _categoryMgr->RegisterGUID(guid, &atom);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_categoryMgr->RegisterGUID failed");

    if (atom == TF_INVALID_GUIDATOM) {
        return E_FAIL;
    }

    CComPtr<ITfProperty> prop;
    hr = context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetProperty failed");

    VARIANT v;
    VariantInit(&v);
    v.vt = VT_I4;
    v.lVal = atom;

    hr = prop->SetValue(ec, range, &v);
    HRESULT_CHECK_RETURN(hr, L"%s", L"prop->SetValue failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::ClearRangeDisplayAttribute(TfEditCookie ec, ITfContext * context, ITfRange * range) {
    HRESULT hr;

    CComPtr<ITfProperty> prop;
    hr = context->GetProperty(GUID_PROP_ATTRIBUTE, &prop);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetProperty failed");

    hr = prop->Clear(ec, range);
    HRESULT_CHECK_RETURN(hr, L"%s", L"prop->Clear failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::_StartComposition(TfEditCookie ec, CompositionManager *instance, ITfContext *context) {
    return instance->StartCompositionNow(ec, context);
}

HRESULT VietType::CompositionManager::_EndComposition(TfEditCookie ec, CompositionManager *instance, ITfContext *context) {
    return instance->EndCompositionNow(ec);
}
