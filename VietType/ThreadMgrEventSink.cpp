// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "Globals.h"
#include "IMECore.h"

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnInitDocumentMgr
//
// Sink called by the framework just before the first context is pushed onto
// a document.
//----------------------------------------------------------------------------

STDAPI IMECore::OnInitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr) {
    pDocMgr;
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnUninitDocumentMgr
//
// Sink called by the framework just after the last context is popped off a
// document.
//----------------------------------------------------------------------------

STDAPI IMECore::OnUninitDocumentMgr(_In_ ITfDocumentMgr *pDocMgr) {
    pDocMgr;
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnSetFocus
//
// Sink called by the framework when focus changes from one document to
// another.  Either document may be NULL, meaning previously there was no
// focus document, or now no document holds the input focus.
//----------------------------------------------------------------------------

STDAPI
IMECore::OnSetFocus(_In_ ITfDocumentMgr *pDocMgrFocus, _In_ ITfDocumentMgr *pDocMgrPrevFocus) {
    pDocMgrPrevFocus;

    _InitTextEditSink(pDocMgrFocus);

    if (_pDocMgrLastFocused) {
        _pDocMgrLastFocused->Release();
        _pDocMgrLastFocused = nullptr;
    }

    _pDocMgrLastFocused = pDocMgrFocus;

    if (_pDocMgrLastFocused) {
        _pDocMgrLastFocused->AddRef();
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPushContext
//
// Sink called by the framework when a context is pushed.
//----------------------------------------------------------------------------

STDAPI IMECore::OnPushContext(_In_ ITfContext *pContext) {
    pContext;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// ITfThreadMgrEventSink::OnPopContext
//
// Sink called by the framework when a context is popped.
//----------------------------------------------------------------------------

STDAPI IMECore::OnPopContext(_In_ ITfContext *pContext) {
    pContext;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// _InitThreadMgrEventSink
//
// Advise our sink.
//----------------------------------------------------------------------------

BOOL IMECore::_InitThreadMgrEventSink() {
    ITfSource *pSource = nullptr;
    BOOL ret = FALSE;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource))) {
        return ret;
    }

    if (FAILED(pSource->AdviseSink(
        IID_ITfThreadMgrEventSink,
        (ITfThreadMgrEventSink *)this,
        &_threadMgrEventSinkCookie))) {
        _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    ret = TRUE;

Exit:
    pSource->Release();
    return ret;
}

//+---------------------------------------------------------------------------
//
// _UninitThreadMgrEventSink
//
// Unadvise our sink.
//----------------------------------------------------------------------------

void IMECore::_UninitThreadMgrEventSink() {
    ITfSource *pSource = nullptr;

    if (_threadMgrEventSinkCookie == TF_INVALID_COOKIE) {
        return;
    }

    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource))) {
        pSource->UnadviseSink(_threadMgrEventSinkCookie);
        pSource->Release();
    }

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
