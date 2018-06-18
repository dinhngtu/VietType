#include "IMECore.h"
#include "EditSessionBase.h"

class EndCompositionEditSession :
    public EditSessionBase {
public:
    EndCompositionEditSession(IMECore *pTextService, ITfContext *pContext) : EditSessionBase(pTextService, pContext) {
    }

    // Inherited via EditSessionBase
    STDMETHODIMP DoEditSession(TfEditCookie ec) {
        return _pTextService->_TerminateComposition(ec, _pContext, TRUE);
    }
};

HRESULT IMECore::_EndComposition(_In_opt_ ITfContext *pContext) {
    EndCompositionEditSession *pEditSession = new (std::nothrow) EndCompositionEditSession(this, pContext);
    HRESULT hr = S_OK, hrSession = S_OK;

    if (nullptr != pEditSession) {
        hr = pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
        pEditSession->Release();
    }

    return hr;
}

HRESULT IMECore::_TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate) {
    isCalledFromDeactivate;

    HRESULT hr = S_OK;

    _engine.Reset();

    if (_pComposition != nullptr) {
        hr = _MoveCaretToEnd(ec);
        if (FAILED(hr)) {
            DBGPRINT(L"cannot move caret: error %lx", hr);
        }

        // remove the display attribute from the composition range.
        //_ClearCompositionDisplayAttributes(ec, pContext);

        hr = _pComposition->EndComposition(ec);
        if (FAILED(hr)) {
            // if we fail to EndComposition, then we need to close the reverse reading window.
            //_DeleteCandidateList(TRUE, pContext);
            DBGPRINT(L"cannot end composition: error %lx", hr);
        }

        _pComposition->Release();
        _pComposition = nullptr;

        if (_pContext) {
            _pContext->Release();
            _pContext = nullptr;
        }
    }

    return hr;
}
