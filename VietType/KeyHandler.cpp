#include "IMECore.h"

BOOL IMECore::_IsRangeCovered(TfEditCookie ec, _In_ ITfRange *pRangeTest, _In_ ITfRange *pRangeCover) {
    LONG lResult = 0;;

    if (FAILED(pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult))
        || (lResult > 0)) {
        return FALSE;
    }

    if (FAILED(pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult))
        || (lResult < 0)) {
        return FALSE;
    }

    return TRUE;
}

STDMETHODIMP IMECore::_SetCompositionText(TfEditCookie ec, ITfContext * pContext, std::wstring content) {
    ITfRange* pRangeComposition = nullptr;
    HRESULT hr = E_FAIL;

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing()) {
        _StartComposition(pContext);
    }

    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition))) {
        pRangeComposition->SetText(ec, TF_ST_CORRECTION, &content[0], (LONG)content.length());
        pRangeComposition->Release();
    }

    _MoveCaretToEnd(ec);

    return hr;
}
