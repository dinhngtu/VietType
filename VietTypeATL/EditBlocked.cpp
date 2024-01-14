// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "EditSessions.h"
#include "EngineController.h"
#include "Compartment.h"
#include "CompositionManager.h"

namespace VietType {
namespace EditSessions {

// workaround for annoying IntelliSense error when the full enum name is used in a macro
using BlockedKind = EngineController::BlockedKind;

#define EB_HRESULT_CHECK_COMMIT(hr, controller, fmt, msg)                                                              \
    do {                                                                                                               \
        if (FAILED(hr)) {                                                                                              \
            DPRINT(L"HRESULT error %lx: " fmt, hr, msg);                                                               \
            controller->SetBlocked(BlockedKind::Free);                                                                 \
            return hr;                                                                                                 \
        }                                                                                                              \
    } while (0);

HRESULT EditBlocked(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller) {

    HRESULT hr;

    DBG_DPRINT(L"EditBlocked ec = %ld", ec);

    // check GUID_COMPARTMENT_KEYBOARD_DISABLED

    Compartment<long> compDisabled;
    hr = compDisabled.Initialize(context, compositionManager->GetClientId(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"compDisabled.Initialize failed");

    long contextDisabled;
    hr = compDisabled.GetValue(&contextDisabled);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"compDisabled.GetValue failed");

    if (hr == S_OK && contextDisabled) {
        DBG_DPRINT(L"%s", L"scopeBlocked: context disabled");
        controller->SetBlocked(BlockedKind::Blocked);
        return S_OK;
    }

    // check transitory context

    TF_STATUS st;
    hr = context->GetStatus(&st);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"context->GetStatus failed");
    if (st.dwStaticFlags & TF_SS_TRANSITORY) {
        // transitory context doesn't seem to support input scopes, free right away
        DBG_DPRINT(L"%s", L"free: transitory context");
        controller->SetBlocked(BlockedKind::Free);
        return S_OK;
    }

    // check input scopes

    CComPtr<ITfReadOnlyProperty> prop;
    hr = context->GetAppProperty(Globals::GUID_PROP_INPUTSCOPE, &prop);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"context->GetAppProperty failed");

    if (!prop) {
        DBG_DPRINT(L"%s", L"GUID_PROP_INPUTSCOPE prop is null, blocking");
        controller->SetBlocked(BlockedKind::Blocked);
        return S_OK;
    }

    TF_SELECTION sel;
    ULONG fetched;
    hr = context->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &fetched);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"context->GetSelection failed");

    CComPtr<ITfRange> range;
    range.Attach(sel.range);

    CComVariant var;
    hr = prop->GetValue(ec, range, &var);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"prop->GetValue failed");

    if (var.vt != VT_UNKNOWN) {
        hr = E_NOINTERFACE;
        EB_HRESULT_CHECK_COMMIT(hr, controller, L"bad variant type %d", static_cast<int>(var.vt));
    }

    CComPtr<ITfInputScope> iis;
    hr = var.punkVal->QueryInterface(&iis);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"var.punkVal->QueryInterface failed");

    CComHeapPtr<InputScope> pscopes;
    UINT scount;
    hr = iis->GetInputScopes(&pscopes, &scount);
    EB_HRESULT_CHECK_COMMIT(hr, controller, L"%s", L"iis->GetInputScopes failed");

    BlockedKind scopeBlocked;
    InputScope* scopes = pscopes;
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
            scopeBlocked = BlockedKind::Blocked;
            goto commit;
        }
    }
    scopeBlocked = BlockedKind::Free;

    hr = S_OK;

commit:
    DBG_DPRINT(L"setting scopeBlocked %d", scopeBlocked);
    controller->SetBlocked(scopeBlocked);
    return hr;
}

} // namespace EditSessions
} // namespace VietType
