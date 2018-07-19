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

HRESULT VietType::EditBlocked(
    TfEditCookie ec,
    VietType::CompositionManager *compositionManager,
    ITfContext *context,
    VietType::EngineController *controller) {

    HRESULT hr;
    VietType::BlockedKind blocked;

    DBG_DPRINT(L"ec = %ld", ec);

    controller->SetEditBlockedPending(S_FALSE);

    // check GUID_COMPARTMENT_KEYBOARD_OPENCLOSE from EngineController

    long openclose;
    hr = controller->GetOpenClose(&openclose);
    HRESULT_CHECK_RETURN(hr, L"%s", L"controller->GetOpenClose failed");

    if (hr == S_OK && openclose) {
        blocked = VietType::BlockedKind::BLOCKED;
        controller->SetBlocked(blocked);
        return S_OK;
    }

    // check if context is contextEmpty

    SmartComObjPtr<Compartment> compEmpty;
    hr = compEmpty.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"compEmpty.CreateInstance failed");

    hr = compEmpty->Initialize(context, compositionManager->GetClientId(), GUID_COMPARTMENT_EMPTYCONTEXT);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compEmpty->Initialize failed");

    long contextEmpty;
    hr = compEmpty->GetValue(&contextEmpty);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->GetValue failed");

    if (hr == S_OK && contextEmpty) {
        blocked = VietType::BlockedKind::BLOCKED;
        controller->SetBlocked(blocked);
        return S_OK;
    }

    // check GUID_COMPARTMENT_KEYBOARD_DISABLED

    SmartComObjPtr<Compartment> compDisabled;
    hr = compDisabled.CreateInstance();
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled.CreateInstance failed");

    hr = compDisabled->Initialize(context, compositionManager->GetClientId(), GUID_COMPARTMENT_KEYBOARD_DISABLED);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->Initialize failed");

    long contextDisabled;
    hr = compDisabled->GetValue(&contextDisabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->GetValue failed");

    if (hr == S_OK && contextDisabled) {
        blocked = VietType::BlockedKind::BLOCKED;
        controller->SetBlocked(blocked);
        return S_OK;
    }

    // check input scopes

    SmartComPtr<ITfReadOnlyProperty> prop;
    hr = context->GetAppProperty(VietType::Globals::GUID_PROP_INPUTSCOPE, prop.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetAppProperty failed");

    TF_SELECTION sel;
    ULONG fetched;
    context->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &sel, &fetched);
    HRESULT_CHECK_RETURN(hr, L"%s", L"context->GetSelection failed");

    VARIANT var;
    hr = prop->GetValue(ec, sel.range, &var);
    HRESULT_CHECK_RETURN(hr, L"%s", L"prop->GetValue failed");

    if (var.vt != VT_UNKNOWN) {
        DBG_DPRINT(L"bad variant type %d", static_cast<int>(var.vt));
        return E_NOINTERFACE;
    }

    SmartComPtr<ITfInputScope> iis(var.punkVal);
    if (!iis) {
        DBG_DPRINT(L"QI on ITfInputScope failed");
        return E_NOINTERFACE;
    }

    CComHeapPtr<InputScope> pscopes;
    UINT scount;
    hr = iis->GetInputScopes(&pscopes, &scount);
    HRESULT_CHECK_RETURN(hr, L"%s", L"iis->GetInputScopes failed");

    InputScope *scopes = pscopes;
    for (UINT i = 0; i < scount; i++) {
        switch (scopes[i]) {
        case IS_EMAIL_SMTPEMAILADDRESS:
        case IS_EMAIL_USERNAME:
        case IS_LOGINNAME:
        case IS_PASSWORD:
            blocked = VietType::BlockedKind::BLOCKED;
            goto commit;
            //case IS_URL:
            //blocked = VietType::BlockedKind::ADVISED;
            //goto commit;
        }
    }
    blocked = VietType::BlockedKind::FREE;

commit:
    DBG_DPRINT(L"setting blocked %d", blocked);
    controller->SetBlocked(blocked);
    return S_OK;
}
