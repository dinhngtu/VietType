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
        _pTextService->_EndComposition(_pContext);
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
    switch (Telex::PushKey(_engine, _wParam, _lParam, _keyState)) {
    case Telex::TELEX_STATES::VALID: {
        auto str = _engine.Peek();
        _pTextService->_SetCompositionText(ec, _pContext, str);
        if (!_engine.Count()) {
            _pTextService->_EndComposition(_pContext);
        }
        break;
    }

    case Telex::TELEX_STATES::INVALID: {
        auto str = _engine.RetrieveInvalid();
        _pTextService->_SetCompositionText(ec, _pContext, str);
        break;
    }

    default:
        DBGPRINT(L"%s", L"PushChar returned unexpected value");
        //throw std::exception("PushChar returned unexpected value");
        break;
    }
}

void KeyHandlerEditSession::Commit(TfEditCookie ec) {
    DBGPRINT(L"%s", L"Committing");
    switch (_engine.Commit()) {
    case Telex::TELEX_STATES::COMMITTED: {
        auto str = _engine.Retrieve();
        _pTextService->_SetCompositionText(ec, _pContext, str);
        break;
    }

    case Telex::TELEX_STATES::COMMITTED_INVALID: {
        auto str = _engine.RetrieveInvalid();
        _pTextService->_SetCompositionText(ec, _pContext, str);
        break;
    }

    default:
        //throw std::exception("Commit returned unexpected value");
        DBGPRINT(L"%s", L"Commit returned unexpected value");
        break;
    }

    _pTextService->_EndComposition(_pContext);
}
