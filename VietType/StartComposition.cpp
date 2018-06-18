#include "IMECore.h"
#include "EditSessionBase.h"

class StartCompositionEditSession : public EditSessionBase {
public:
    StartCompositionEditSession(_In_ IMECore *pTextService, _In_ ITfContext *pContext) : EditSessionBase(pTextService, pContext) {
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec) {
        ITfInsertAtSelection* pInsertAtSelection = nullptr;
        ITfRange* pRangeInsert = nullptr;
        ITfContextComposition* pContextComposition = nullptr;
        ITfComposition* pComposition = nullptr;
        HRESULT hr = S_OK;

        hr = _pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = _pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition);
        if (FAILED(hr)) {
            goto Exit;
        }

        hr = pContextComposition->StartComposition(ec, pRangeInsert, _pTextService, &pComposition);
        if (SUCCEEDED(hr) && (nullptr != pComposition)) {
            _pTextService->_SetComposition(pComposition);

            // set selection to the adjusted range
            TF_SELECTION tfSelection;
            tfSelection.range = pRangeInsert;
            tfSelection.style.ase = TF_AE_NONE;
            tfSelection.style.fInterimChar = FALSE;

            _pContext->SetSelection(ec, 1, &tfSelection);
            _pTextService->_SaveCompositionContext(_pContext);
        }

    Exit:
        if (nullptr != pContextComposition) {
            pContextComposition->Release();
        }

        if (nullptr != pRangeInsert) {
            pRangeInsert->Release();
        }

        if (nullptr != pInsertAtSelection) {
            pInsertAtSelection->Release();
        }

        return hr;
    }
};

HRESULT IMECore::_StartComposition(_In_ ITfContext *pContext) {
    StartCompositionEditSession* pStartCompositionEditSession = new (std::nothrow) StartCompositionEditSession(this, pContext);
    HRESULT hr = S_OK, hrSession = S_OK;

    if (nullptr != pStartCompositionEditSession) {
        // note that SampleIME uses TF_ES_SYNC
        hr = pContext->RequestEditSession(_tfClientId, pStartCompositionEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);

        pStartCompositionEditSession->Release();
    }

    return hr;
}

void IMECore::_SaveCompositionContext(_In_ ITfContext *pContext) {
    assert(_pContext == nullptr);

    pContext->AddRef();
    _pContext = pContext;
}
