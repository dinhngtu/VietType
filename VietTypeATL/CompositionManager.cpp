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

    hr = EndComposition();
    DBG_HRESULT_CHECK(hr, L"%s", L"EndComposition failed");

    return S_OK;
}

void VietType::CompositionManager::Initialize(TfClientId clientid) {
    _clientid = clientid;
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
    return RequestEditSession(&_StartComposition, this, pContext);
}

HRESULT VietType::CompositionManager::EndComposition() {
    assert(_clientid != TF_CLIENTID_NULL);
    return RequestEditSession(&_EndComposition, this, _context);
}

bool VietType::CompositionManager::IsComposing() const {
    return (bool)_composition;
}

HRESULT VietType::CompositionManager::EmptyCompositionText(TfEditCookie ec) {
    HRESULT hr;

    SmartComPtr<ITfRange> range;
    hr = _composition->GetRange(range.GetAddress());
    if (SUCCEEDED(hr)) {
        range->SetText(ec, 0, NULL, 0);
    } else HRESULT_CHECK_RETURN(hr, L"%s", L"composition->GetRange failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::MoveCaretToEnd(TfEditCookie ec) {
    HRESULT hr;

    if (_composition) {
        SmartComPtr<ITfRange> range;
        hr = _composition->GetRange(range.GetAddress());
        HRESULT_CHECK_RETURN(hr, L"%s", L"_composition->GetRange failed");

        SmartComPtr<ITfRange> rangeClone;
        hr = range->Clone(rangeClone.GetAddress());
        HRESULT_CHECK_RETURN(hr, L"%s", L"range->Clone failed");

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
        SmartComPtr<ITfRange> range;
        hr = _composition->GetRange(range.GetAddress());
        HRESULT_CHECK_RETURN(hr, L"%s", L"_composition->GetRange failed");
        hr = range->SetText(ec, TF_ST_CORRECTION, str, length);
        HRESULT_CHECK_RETURN(hr, L"%s", L"range->SetText failed");
        hr = MoveCaretToEnd(ec);
        DBG_HRESULT_CHECK(hr, L"%s", L"MoveCaretToEnd failed");
    }

    return S_OK;
}

HRESULT VietType::CompositionManager::EnsureCompositionText(ITfContext *context, TfEditCookie ec, WCHAR const * str, LONG length) {
    HRESULT hr;

    if (!_composition) {
        hr = StartComposition(context);
        HRESULT_CHECK_RETURN(hr, L"%s", L"StartComposition failed");
    }

    return SetCompositionText(ec, str, length);
}

HRESULT VietType::CompositionManager::_StartComposition(TfEditCookie ec, CompositionManager *instance, ITfContext *context) {
    HRESULT hr;

    SmartComPtr<ITfCompositionSink> compositionSink(instance);
    if (!compositionSink) {
        return E_NOINTERFACE;
    }

    SmartComPtr<ITfInsertAtSelection> insertAtSelection(context);
    if (!insertAtSelection) {
        return E_NOINTERFACE;
    }

    SmartComPtr<ITfRange> insertRange;
    hr = insertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, insertRange.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"insertAtSelection->InsertTextAtSelection failed");

    SmartComPtr<ITfContextComposition> contextComposition(context);
    if (!contextComposition) {
        return E_NOINTERFACE;
    }

    ITfComposition *composition;
    hr = contextComposition->StartComposition(ec, insertRange, compositionSink, &composition);
    if (SUCCEEDED(hr)) {
        instance->_composition = composition;

        TF_SELECTION sel;
        sel.range = insertRange;
        sel.style.ase = TF_AE_NONE;
        sel.style.fInterimChar = FALSE;
        hr = context->SetSelection(ec, 1, &sel);
        DBG_HRESULT_CHECK(hr, L"%s", L"context->SetSelection failed");

        instance->_context = context;
    } else HRESULT_CHECK_RETURN(hr, L"%s", L"contextComposition->StartComposition failed");

    return S_OK;
}

HRESULT VietType::CompositionManager::_EndComposition(TfEditCookie ec, CompositionManager *instance, ITfContext *context) {
    return instance->EndCompositionNow(ec);
}
