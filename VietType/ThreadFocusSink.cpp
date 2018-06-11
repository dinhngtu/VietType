// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "IMECore.h"

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink::OnSetThreadFocus
//
//----------------------------------------------------------------------------

STDAPI IMECore::OnSetThreadFocus() {
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfTextLayoutSink::OnKillThreadFocus
//
//----------------------------------------------------------------------------

STDAPI IMECore::OnKillThreadFocus() {
    return S_OK;
}

BOOL IMECore::_InitThreadFocusSink() {
    ITfSource *pSource = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource))) {
        return FALSE;
    }

    if (FAILED(pSource->AdviseSink(
        IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &_dwThreadFocusSinkCookie))) {
        pSource->Release();
        return FALSE;
    }

    pSource->Release();

    return TRUE;
}

void IMECore::_UninitThreadFocusSink() {
    ITfSource *pSource = nullptr;

    if (FAILED(_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource))) {
        return;
    }

    if (FAILED(pSource->UnadviseSink(_dwThreadFocusSinkCookie))) {
        pSource->Release();
        return;
    }

    pSource->Release();
}
