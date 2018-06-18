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
    HRESULT hr = S_OK;

    // Clear dummy composition
    hr = _RemoveDummyCompositionForComposing(ecWrite, pComposition);
    if (FAILED(hr)) {
        DBGPRINT(L"cannot clear dummy composition: error %lx", hr);
    }

    // Clear display attribute and end composition, _EndComposition will release composition for us
    ITfContext* pContext = _pContext;
    if (pContext) {
        pContext->AddRef();
    }

    hr = _EndComposition(_pContext);
    if (FAILED(hr)) {
        DBGPRINT(L"cannot end composition: error %lx", hr);
    }

    if (pContext) {
        pContext->Release();
        pContext = nullptr;
    }

    return hr;
}

void IMECore::_SetComposition(_In_ ITfComposition *pComposition) {
    _pComposition = pComposition;
}

bool IMECore::_IsComposing() const {
    return _pComposition != nullptr;
}

HRESULT IMECore::_MoveCaretToEnd(TfEditCookie ec) {
    HRESULT hr;

    if (_pComposition != nullptr) {
        ITfRange* pRangeComposition = nullptr;
        hr = _pComposition->GetRange(&pRangeComposition);

        if (SUCCEEDED(hr)) {
            ITfRange* pRangeCloned = nullptr;
            hr = pRangeComposition->Clone(&pRangeCloned);

            if (SUCCEEDED(hr)) {
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

    return hr;
}
