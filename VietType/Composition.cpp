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
