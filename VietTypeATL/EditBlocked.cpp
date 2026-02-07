// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "Compartment.h"

namespace VietType {

#define EB_HRESULT_CHECK_COMMIT(hr, context, fmt, ...)                                                                 \
    do {                                                                                                               \
        if (FAILED(hr)) {                                                                                              \
            DPRINT(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__);                                                       \
            context->_blocked = false;                                                                                 \
            return hr;                                                                                                 \
        }                                                                                                              \
    } while (0);

HRESULT Context::EditBlocked(_In_ TfEditCookie ec, _In_ Context* context) {

    HRESULT hr;

    DBG_DPRINT(L"EditBlocked ec = %ld", ec);

    // check GUID_COMPARTMENT_KEYBOARD_DISABLED

    Compartment<long> compDisabled;
    hr = compDisabled.Initialize(context->GetContext(), context->GetClientId(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"compDisabled.Initialize failed");

    long contextDisabled;
    hr = compDisabled.GetValue(&contextDisabled);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"compDisabled.GetValue failed");

    if (hr == S_OK && contextDisabled) {
        DBG_DPRINT(L"scopeBlocked: context disabled");
        context->_blocked = true;
        return S_OK;
    }

    // check transitory context

    TF_STATUS st;
    hr = context->GetContext()->GetStatus(&st);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"context->GetStatus failed");
    if (st.dwStaticFlags & TF_SS_TRANSITORY) {
        // transitory context doesn't seem to support input scopes, free right away
        DBG_DPRINT(L"free: transitory context");
        context->_blocked = false;
        return S_OK;
    }

    // check input scopes

    CComPtr<ITfReadOnlyProperty> prop;
    hr = context->GetContext()->GetAppProperty(Globals::GUID_PROP_INPUTSCOPE, &prop);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"context->GetAppProperty failed");

    if (!prop) {
        DBG_DPRINT(L"GUID_PROP_INPUTSCOPE prop is null, blocking");
        context->_blocked = true;
        return S_OK;
    }

    TF_SELECTION sel;
    ULONG fetched;
    hr = context->GetContext()->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &fetched);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"context->GetSelection failed");

    CComPtr<ITfRange> range;
    range.Attach(sel.range);

    CComVariant var;
    hr = prop->GetValue(ec, range, &var);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"prop->GetValue failed");

    if (var.vt != VT_UNKNOWN) {
        hr = E_NOINTERFACE;
        EB_HRESULT_CHECK_COMMIT(hr, context, L"bad variant type %d", static_cast<int>(var.vt));
    }

    CComPtr<ITfInputScope> iis;
    hr = var.punkVal->QueryInterface(&iis);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"var.punkVal->QueryInterface failed");

    CComHeapPtr<InputScope> pscopes;
    UINT scount;
    hr = iis->GetInputScopes(&pscopes, &scount);
    EB_HRESULT_CHECK_COMMIT(hr, context, L"iis->GetInputScopes failed");

    bool scopeBlocked;
    InputScope* scopes = pscopes;
#ifdef _DEBUG
    for (UINT i = 0; i < scount; i++) {
        DBG_DPRINT(L"scope: %u", scopes[i]);
    }
#endif
    for (UINT i = 0; i < scount; i++) {
        switch (scopes[i]) {
        case IS_EMAIL_USERNAME:
        case IS_EMAIL_SMTPEMAILADDRESS:
        case IS_LOGINNAME:
        case IS_PASSWORD:
        case IS_EMAILNAME_OR_ADDRESS:
        case IS_NUMERIC_PASSWORD:
        case IS_NUMERIC_PIN:
        case IS_ALPHANUMERIC_PIN:
        case IS_ALPHANUMERIC_PIN_SET:
            scopeBlocked = true;
            goto commit;
        }
    }
    scopeBlocked = false;

    hr = S_OK;

commit:
    DBG_DPRINT(L"setting scope to %s", scopeBlocked ? "blocked" : "free");
    context->_blocked = scopeBlocked;
    return hr;
}

} // namespace VietType
