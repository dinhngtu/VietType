// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "IMECore.h"
#include "globals.h"

//+---------------------------------------------------------------------------
//
// ITfTextEditSink::OnEndEdit
//
// Called by the system whenever anyone releases a write-access document lock.
//----------------------------------------------------------------------------

STDAPI IMECore::OnEndEdit(
    __RPC__in_opt ITfContext *pContext,
    TfEditCookie ecReadOnly,
    __RPC__in_opt ITfEditRecord *pEditRecord) {

    //
    // did the selection change?
    // The selection change includes the movement of caret as well.
    // The caret position is represent as the empty selection range when
    // there is no selection.
    //
    if (pEditRecord == nullptr) {
        return E_INVALIDARG;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitTextEditSink
//
// Init a text edit sink on the topmost context of the document.
// Always release any previous sink.
//----------------------------------------------------------------------------

BOOL IMECore::_InitTextEditSink(_In_ ITfDocumentMgr *pDocMgr) {
    ITfSource *pSource = nullptr;
    BOOL ret = TRUE;

    // clear out any previous sink first
    if (_textEditSinkCookie != TF_INVALID_COOKIE) {
        if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource))) {
            pSource->UnadviseSink(_textEditSinkCookie);
            pSource->Release();
        }

        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = nullptr;
        _textEditSinkCookie = TF_INVALID_COOKIE;
    }

    if (pDocMgr == nullptr) {
        return TRUE; // caller just wanted to clear the previous sink
    }

    if (FAILED(pDocMgr->GetTop(&_pTextEditSinkContext))) {
        return FALSE;
    }

    if (_pTextEditSinkContext == nullptr) {
        return TRUE; // empty document, no sink possible
    }

    ret = FALSE;
    if (SUCCEEDED(_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource))) {
        if (SUCCEEDED(pSource->AdviseSink(
            IID_ITfTextEditSink, (ITfTextEditSink *)this, &_textEditSinkCookie))) {
            ret = TRUE;
        } else {
            _textEditSinkCookie = TF_INVALID_COOKIE;
        }
        pSource->Release();
    }

    if (ret == FALSE) {
        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = nullptr;
    }

    return ret;
}
