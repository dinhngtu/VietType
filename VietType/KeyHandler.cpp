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
    TF_SELECTION tfSelection;
    ULONG fetched = 0;
    BOOL isCovered = TRUE;
    HRESULT hr = E_FAIL;

    // Start the new (std::nothrow) compositon if there is no composition.
    if (!_IsComposing()) {
        _StartComposition(pContext);
    }

    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition))) {
        pRangeComposition->SetText(ec, TF_ST_CORRECTION, &content[0], (long)content.length());
    }

    /*
    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &fetched) != S_OK || fetched != 1) {
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition))) {
        isCovered = _IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!isCovered) {
            goto Exit;
        }
    }

    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    hr = tfSelection.range->SetText(ec, 0, &content[0], content.length());
    if (hr == S_OK) {
        // update the selection, we'll make it an insertion point just past
        // the inserted text.
        tfSelection.range->Collapse(ec, TF_ANCHOR_END);
        pContext->SetSelection(ec, 1, &tfSelection);
    }

    tfSelection.range->Release();
    */

    return hr;

//Exit:
    tfSelection.range->Release();
    return S_OK;
}
