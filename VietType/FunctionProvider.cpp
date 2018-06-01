// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"
#include "SampleIME.h"

//+---------------------------------------------------------------------------
//
// _InitFunctionProviderSink
//
//----------------------------------------------------------------------------

BOOL CSampleIME::_InitFunctionProviderSink() {
    ITfSourceSingle *pSourceSingle = nullptr;
    BOOL ret = FALSE;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle))) {
        IUnknown *punk = nullptr;
        if (SUCCEEDED(QueryInterface(IID_IUnknown, (void **)&punk))) {
            if (SUCCEEDED(
                    pSourceSingle->AdviseSingleSink(_tfClientId, IID_ITfFunctionProvider, punk))) {
                ret = TRUE;
            }
            punk->Release();
        }
        pSourceSingle->Release();
    }
    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _UninitFunctionProviderSink
//
//----------------------------------------------------------------------------

void CSampleIME::_UninitFunctionProviderSink() {
    ITfSourceSingle *pSourceSingle = nullptr;
    if (SUCCEEDED(_pThreadMgr->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle))) {
        pSourceSingle->UnadviseSingleSink(_tfClientId, IID_ITfFunctionProvider);
        pSourceSingle->Release();
    }
}
