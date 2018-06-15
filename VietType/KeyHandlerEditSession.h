#pragma once

#include "EditSessionBase.h"
#include "Telex.h"

class KeyHandlerEditSession :
    public EditSessionBase {
public:
    KeyHandlerEditSession(IMECore *pTextService, ITfContext *pContext, WPARAM wParam, LPARAM lParam, BYTE const *keyState, Telex::TelexEngine& engine);
    virtual ~KeyHandlerEditSession();

    // Inherited via EditSessionBase
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    void ComposeKey(TfEditCookie ec);
    void Commit(TfEditCookie ec);

private:
    WPARAM _wParam;
    LPARAM _lParam;
    BYTE const *_keyState;
    Telex::TelexEngine& _engine;
};

