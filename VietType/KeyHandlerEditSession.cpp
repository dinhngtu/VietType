#include "KeyHandlerEditSession.h"
#include "TelexKeyTranslator.h"

KeyHandlerEditSession::KeyHandlerEditSession(
    IMECore * pTextService,
    ITfContext * pContext,
    WPARAM wParam,
    LPARAM lParam,
    BYTE const *keyState,
    Telex::TelexEngine& engine)
    : EditSessionBase(pTextService, pContext), _engine(engine) {
    DBGPRINT(L"creating key handler session key %x", wParam);
    _wParam = wParam;
    _lParam = lParam;
    _keyState = keyState;
}

KeyHandlerEditSession::~KeyHandlerEditSession() {
}

STDMETHODIMP KeyHandlerEditSession::DoEditSession(TfEditCookie ec) {
    DBGPRINT(L"%s", L"entering key handler session");

    if (_wParam == 0) {
        Commit(ec);
    } else if (Telex::IsEditKey(_wParam, _lParam, _keyState)) {
        return _pTextService->_TerminateComposition(ec, _pContext, FALSE);
    } else if (Telex::EngineWantsKey(_pTextService->_IsComposing(), _wParam, _lParam, _keyState)) {
        ComposeKey(ec);
    } else if (_wParam == VK_SHIFT) {
        // drop shift
    } else {
        Commit(ec);
    }

    return S_OK;
}

void KeyHandlerEditSession::ComposeKey(TfEditCookie ec) {
    HRESULT hr = S_OK;

    switch (Telex::PushKey(_engine, _wParam, _lParam, _keyState)) {
    case Telex::TELEX_STATES::VALID: {
        auto str = _engine.Peek();
        hr = _pTextService->_SetCompositionText(ec, _pContext, str);
        if (FAILED(hr)) {
            DBGPRINT(L"cannot reset composition text: error %lx", hr);
        }
        // backspace only returns VALID on an empty buffer
        if (!_engine.Count()) {
            hr = _pTextService->_TerminateComposition(ec, _pContext, FALSE);
            if (FAILED(hr)) {
                DBGPRINT(L"cannot terminate composition: error %lx", hr);
            }
        }
        break;
    }

    case Telex::TELEX_STATES::INVALID: {
        assert(_engine.Count() > 0);
        auto str = _engine.RetrieveInvalid();
        hr = _pTextService->_SetCompositionText(ec, _pContext, str);
        if (FAILED(hr)) {
            DBGPRINT(L"cannot reset composition text: error %lx", hr);
        }
        break;
    }

    default:
        DBGPRINT(L"%s", L"PushChar returned unexpected value");
        //throw std::exception("PushChar returned unexpected value");
        break;
    }
}

void KeyHandlerEditSession::Commit(TfEditCookie ec) {
    HRESULT hr = S_OK;

    DBGPRINT(L"%s", L"Committing");
    switch (_engine.Commit()) {
    case Telex::TELEX_STATES::COMMITTED: {
        auto str = _engine.Retrieve();
        hr = _pTextService->_SetCompositionText(ec, _pContext, str);
        if (FAILED(hr)) {
            DBGPRINT(L"cannot reset composition text: error %lx", hr);
        }
        break;
    }

    case Telex::TELEX_STATES::COMMITTED_INVALID: {
        auto str = _engine.RetrieveInvalid();
        hr = _pTextService->_SetCompositionText(ec, _pContext, str);
        if (FAILED(hr)) {
            DBGPRINT(L"cannot reset composition text: error %lx", hr);
        }
        break;
    }

    default:
        //throw std::exception("Commit returned unexpected value");
        DBGPRINT(L"%s", L"Commit returned unexpected value");
        break;
    }

    hr = _pTextService->_TerminateComposition(ec, _pContext, FALSE);
    if (FAILED(hr)) {
        DBGPRINT(L"cannot terminate composition: error %lx", hr);
    }
}
