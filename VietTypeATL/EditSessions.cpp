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

HRESULT VietType::EditBlocked(
    TfEditCookie ec,
    VietType::CompositionManager *compositionManager,
    ITfContext *context,
    VietType::EngineController *controller) {

    HRESULT hr;
    VietType::BlockedKind blocked;

    DBG_DPRINT(L"ec = %ld", ec);

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
    //*editBlockedPending = false;
    return S_OK;
}
