#include "IMECore.h"

HRESULT _RemoveDummyCompositionForComposing(TfEditCookie ec, _In_ ITfComposition *pComposition) {
    HRESULT hr = S_OK;

    ITfRange* pRange = nullptr;

    if (pComposition) {
        hr = pComposition->GetRange(&pRange);
        if (SUCCEEDED(hr)) {
            pRange->SetText(ec, 0, nullptr, 0);
            pRange->Release();
        }
    }

    return hr;
}

STDMETHODIMP IMECore::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition * pComposition) {
    // Clear dummy composition
    _RemoveDummyCompositionForComposing(ecWrite, pComposition);

    // Clear display attribute and end composition, _EndComposition will release composition for us
    ITfContext* pContext = _pContext;
    if (pContext) {
        pContext->AddRef();
    }

    _EndComposition(_pContext);

    if (pContext) {
        pContext->Release();
        pContext = nullptr;
    }

    return S_OK;
}

void IMECore::_SetComposition(_In_ ITfComposition *pComposition) {
    _pComposition = pComposition;
}

bool IMECore::_IsComposing() const {
    return _pComposition != nullptr;
}

void IMECore::_MoveCaretToEnd(TfEditCookie ec) {
    if (_pComposition != nullptr) {
        ITfRange* pRangeComposition = nullptr;
        if (SUCCEEDED(_pComposition->GetRange(&pRangeComposition))) {
            ITfRange* pRangeCloned = nullptr;
            if (SUCCEEDED(pRangeComposition->Clone(&pRangeCloned))) {
                pRangeCloned->Collapse(ec, TF_ANCHOR_END);
                TF_SELECTION sel;
                sel.range = pRangeCloned;
                sel.style.ase = TF_AE_NONE;
                sel.style.fInterimChar = FALSE;
                _pContext->SetSelection(ec, 1, &sel);
                pRangeCloned->Release();
            }
            pRangeComposition->Release();
        }
    }
}
