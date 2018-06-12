#include "KeyHandlerEditSession.h"
#include "TelexKeyTranslator.h"

KeyHandlerEditSession::KeyHandlerEditSession(
    IMECore * pTextService,
    ITfContext * pContext,
    WPARAM wParam,
    LPARAM lParam,
    PBYTE keyState,
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
    WCHAR buf;

    if (_wParam == 0) {
        Commit(ec);
    } else if (Telex::TranslateKey(_wParam, _lParam, _keyState, &buf)) {
        ComposeChar(ec, buf);
    } else {
        Commit(ec);
    }

    return S_OK;
}

void KeyHandlerEditSession::ComposeChar(TfEditCookie ec, WCHAR c) {
    DBGPRINT(L"Translated character '%c'", c);
    switch (_engine.PushChar(c)) {
    case Telex::TELEX_STATES::VALID: {
        auto str = _engine.Retrieve();
        DBGPRINT(L"PushChar valid %s", str.c_str());
        _pTextService->_SetCompositionText(ec, _pContext, str);
        break;
    }

    case Telex::TELEX_STATES::INVALID: {
        auto str = _engine.RetrieveInvalid();
        DBGPRINT(L"PushChar invalid %s", str.c_str());
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
        DBGPRINT(L"Valid %s", str.c_str());
        _pTextService->_SetCompositionText(ec, _pContext, str);
        break;
    }

    case Telex::TELEX_STATES::COMMITTED_INVALID: {
        auto str = _engine.RetrieveInvalid();
        DBGPRINT(L"Invalid %s", str.c_str());
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
