#include "TestEditSession.h"

TestEditSession::TestEditSession(IMECore *pTextService, ITfContext *pContext, WPARAM wParam, LPARAM lParam, PBYTE keyState, HKL hkl)
    : EditSessionBase(pTextService, pContext) {
    _wParam = wParam;
    _lParam = lParam;
    _keyState = keyState;
    _hkl = hkl;
}

STDMETHODIMP TestEditSession::DoEditSession(TfEditCookie ec) {
    ITfInsertAtSelection *pInsertAtSelection = nullptr;
    ITfRange *pRangeInsert = nullptr;
    WCHAR buf[8] = { 0 };
    int count;

    if (!_keyState) {
        goto Exit;
    }

    if (FAILED(_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection))) {
        goto Exit;
    }

    //count = ToUnicodeEx((UINT)_wParam, (_lParam >> 16) & 0xff, _keyState, buf, 8, 0, _hkl);
    count = ToUnicode((UINT)_wParam, (_lParam >> 16) & 0xff, _keyState, buf, 8, 0);
    if (count <= 0) {
        goto Exit;
    }

    if (FAILED(pInsertAtSelection->InsertTextAtSelection(ec, 0, buf, count, &pRangeInsert))) {
        goto Exit;
    }

Exit:
    /*
    if (nullptr != pContextComposition) {
        pContextComposition->Release();
    }
    */

    if (nullptr != pRangeInsert) {
        pRangeInsert->Release();
    }

    if (nullptr != pInsertAtSelection) {
        pInsertAtSelection->Release();
    }

    return S_OK;
}
