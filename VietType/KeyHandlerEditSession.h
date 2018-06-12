#pragma once

#include "EditSessionBase.h"
#include "Telex.h"

class KeyHandlerEditSession :
    public EditSessionBase {
public:
    KeyHandlerEditSession(IMECore *pTextService, ITfContext *pContext, WPARAM wParam, LPARAM lParam, PBYTE keyState, Telex::TelexEngine& engine);
    virtual ~KeyHandlerEditSession();

    // Inherited via EditSessionBase
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    void ComposeChar(TfEditCookie ec, WCHAR c);
    void Commit(TfEditCookie ec);

private:
    WPARAM _wParam;
    LPARAM _lParam;
    PBYTE _keyState;
    Telex::TelexEngine& _engine;
};

