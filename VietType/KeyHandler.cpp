#include "IMECore.h"

BOOL IMECore::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover) {
    LONG lResult = 0;;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult)) || (lResult > 0)) {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult)) || (lResult < 0)) {
        return FALSE;
    }

    return TRUE;
}

STDMETHODIMP IMECore::_SetCompositionText(TfEditCookie ec, ITfContext * pContext, std::wstring content) {
    ITfRange* pRangeComposition = nullptr;
    HRESULT hr = S_OK;

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing()) {
        hr = _StartComposition(pContext);
        if (FAILED(hr)) {
            DBGPRINT(L"start composition failed: error %lx", hr);
        }
    }

    hr = _pComposition->GetRange(&pRangeComposition);
    if (SUCCEEDED(hr)) {
        hr = pRangeComposition->SetText(ec, TF_ST_CORRECTION, &content[0], (LONG)content.length());
        if (FAILED(hr)) {
            DBGPRINT(L"composition set text failed: error %lx", hr);
        }
        pRangeComposition->Release();
    } else {
        DBGPRINT(L"get composition range failed: error %lx", hr);
    }

    hr = _MoveCaretToEnd(ec);
    if (FAILED(hr)) {
        DBGPRINT(L"cannot move caret: error %lx", hr);
    }

    return hr;
}
