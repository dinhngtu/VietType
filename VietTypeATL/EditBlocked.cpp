// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#include "EditSessions.h"
#include "EngineController.h"
#include "Compartment.h"
#include "CompositionManager.h"

HRESULT VietType::EditSessions::EditBlocked(
    _In_ TfEditCookie ec,
    _In_ VietType::CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ VietType::EngineController* controller) {

    HRESULT hr;

    DBG_DPRINT(L"ec = %ld", ec);

    controller->ResetEditBlockedPending();

    // check GUID_COMPARTMENT_KEYBOARD_OPENCLOSE from EngineController

    long openclose;
    hr = controller->GetOpenClose(&openclose);
    HRESULT_CHECK_RETURN(hr, L"%s", L"controller->GetOpenClose failed");

    if (hr == S_OK && openclose) {
        DBG_DPRINT(L"%s", L"blocked: openclose");
        controller->SetBlocked(VietType::EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    // check GUID_COMPARTMENT_KEYBOARD_DISABLED

    CComPtr<Compartment> compDisabled;
    hr = CreateInstance2(&compDisabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInstance2(&compDisabled) failed");

    hr = compDisabled->Initialize(context, compositionManager->GetClientId(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->Initialize failed");

    long contextDisabled;
    hr = compDisabled->GetValue(&contextDisabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->GetValue failed");

    if (hr == S_OK && contextDisabled) {
        DBG_DPRINT(L"%s", L"blocked: context disabled");
        controller->SetBlocked(VietType::EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    // check transitory context

    TF_STATUS st;
    hr = context->GetStatus(&st);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetStatus failed");
    if (st.dwStaticFlags & TF_SS_TRANSITORY) {
        // transitory context doesn't seem to support input scopes, free right away
        DBG_DPRINT(L"%s", L"free: transitory context");
        controller->SetBlocked(VietType::EngineController::BlockedKind::Free);
        return S_OK;
    }

    // check input scopes

    CComPtr<ITfReadOnlyProperty> prop;
    hr = context->GetAppProperty(VietType::Globals::GUID_PROP_INPUTSCOPE, &prop);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetAppProperty failed");

    TF_SELECTION sel;
    ULONG fetched;
    hr = context->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &fetched);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetSelection failed");

    VARIANT var;
    hr = prop->GetValue(ec, sel.range, &var);
    HRESULT_CHECK_RETURN(hr, L"%s", L"prop->GetValue failed");

    if (var.vt != VT_UNKNOWN) {
        DBG_DPRINT(L"bad variant type %d", static_cast<int>(var.vt));
        controller->SetBlocked(VietType::EngineController::BlockedKind::Free);
        return E_NOINTERFACE;
    }

    CComPtr<ITfInputScope> iis;
    hr = var.punkVal->QueryInterface(&iis);
    HRESULT_CHECK_RETURN(hr, L"%s", L"var.punkVal->QueryInterface failed");

    CComHeapPtr<InputScope> pscopes;
    UINT scount;
    hr = iis->GetInputScopes(&pscopes, &scount);
    HRESULT_CHECK_RETURN(hr, L"%s", L"iis->GetInputScopes failed");

    VietType::EngineController::BlockedKind blocked;
    InputScope* scopes = pscopes;
    for (UINT i = 0; i < scount; i++) {
        switch (scopes[i]) {
        case IS_EMAIL_SMTPEMAILADDRESS:
        case IS_EMAIL_USERNAME:
        case IS_LOGINNAME:
        case IS_PASSWORD:
            blocked = VietType::EngineController::BlockedKind::Blocked;
            goto commit;
        }
    }
    blocked = VietType::EngineController::BlockedKind::Free;

commit:
    DBG_DPRINT(L"setting blocked %d", blocked);
    controller->SetBlocked(blocked);
    return S_OK;
}
