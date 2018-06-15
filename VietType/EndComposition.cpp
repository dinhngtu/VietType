#include "IMECore.h"
#include "EditSessionBase.h"

class EndCompositionEditSession :
    public EditSessionBase {
public:
    EndCompositionEditSession(IMECore *pTextService, ITfContext *pContext) : EditSessionBase(pTextService, pContext) {
    }

    // Inherited via EditSessionBase
    STDMETHODIMP DoEditSession(TfEditCookie ec) {
        _pTextService->_TerminateComposition(ec, _pContext, TRUE);
        return S_OK;
    }
};

void IMECore::_EndComposition(_In_opt_ ITfContext *pContext) {
    EndCompositionEditSession *pEditSession = new (std::nothrow) EndCompositionEditSession(this, pContext);
    HRESULT hr = S_OK;

    if (nullptr != pEditSession) {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

void IMECore::_TerminateComposition(TfEditCookie ec, _In_ ITfContext *pContext, BOOL isCalledFromDeactivate) {
    isCalledFromDeactivate;

    _engine.Reset();

    if (_pComposition != nullptr) {
        _MoveCaretToEnd(ec);

        // remove the display attribute from the composition range.
        //_ClearCompositionDisplayAttributes(ec, pContext);

        if (FAILED(_pComposition->EndComposition(ec))) {
            // if we fail to EndComposition, then we need to close the reverse reading window.
            //_DeleteCandidateList(TRUE, pContext);
            DBGPRINT(L"%s", L"cannot end composition");
        }

        _pComposition->Release();
        _pComposition = nullptr;

        if (_pContext) {
            _pContext->Release();
            _pContext = nullptr;
        }
    }
}
