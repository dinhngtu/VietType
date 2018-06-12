#pragma once

#include "EditSessionBase.h"

class TestEditSession : public EditSessionBase {
public:
    TestEditSession(
        IMECore *pTextService,
        ITfContext *pContext,
        WPARAM wParam,
        LPARAM lParam,
        PBYTE keyState,
        HKL hkl);

    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    WPARAM _wParam;
    LPARAM _lParam;
    PBYTE _keyState;
    HKL _hkl;
};
