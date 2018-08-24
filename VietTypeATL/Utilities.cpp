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

#include "Utilities.h"
#include "Compartment.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "EditSessions.h"

namespace VietType {

HRESULT IsContextEmpty(_In_ ITfContext* context, _In_ TfClientId clientid, _Out_ bool* isempty) {
    HRESULT hr;

    Compartment<long> compEmpty;
    hr = compEmpty.Initialize(context, clientid, GUID_COMPARTMENT_EMPTYCONTEXT);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compEmpty->Initialize failed");

    long contextEmpty;
    hr = compEmpty.GetValue(&contextEmpty);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compDisabled->GetValue failed");

    *isempty = hr == S_OK && contextEmpty;
    return hr;
}

HRESULT OnNewContext(_In_ ITfContext *context, _In_ CompositionManager* compositionManager, _In_ EngineController* controller) {
    HRESULT hr;

    bool isempty;
    hr = IsContextEmpty(context, compositionManager->GetClientId(), &isempty);
    HRESULT_CHECK_RETURN(hr, L"%s", L"IsContextEmpty failed");
    if (isempty) {
        controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

#ifdef _DEBUG
    TF_STATUS st;
    hr = context->GetStatus(&st);
    if (SUCCEEDED(hr)) {
        DBG_DPRINT(
            L"d=%c%c%c s=%c%c%c",
            (st.dwDynamicFlags & TF_SD_LOADING) ? L'L' : L'_',
            (st.dwDynamicFlags & TF_SD_READONLY) ? L'R' : L'_',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'U' : L'_',
            (st.dwStaticFlags & TF_SS_DISJOINTSEL) ? L'D' : L'_',
            (st.dwStaticFlags & TF_SS_REGIONS) ? L'R' : L'_',
            (st.dwStaticFlags & TF_SS_TRANSITORY) ? L'T' : L'_');
    } else DBG_HRESULT_CHECK(hr, L"%s", L"context->GetStatus failed");
#endif

    hr = CompositionManager::RequestEditSession(
        EditSessions::EditBlocked,
        compositionManager,
        context,
        controller);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"CompositionManager::RequestEditSession failed");
        controller->SetBlocked(EngineController::BlockedKind::Free);
    }

    return S_OK;
}

}
