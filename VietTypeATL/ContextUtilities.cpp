// SPDX-License-Identifier: GPL-3.0-only

#include "ContextUtilities.h"
#include "Compartment.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "EditSessions.h"

namespace VietType {

static HRESULT IsContextEmpty(_In_ ITfContext* context, _In_ TfClientId clientid, _Out_ bool* isempty) {
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

HRESULT OnNewContext(
    _In_opt_ ITfContext* context, _In_ CompositionManager* compositionManager, _In_ EngineController* controller) {
    HRESULT hr;

    if (!context) {
        controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

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
    DBG_HRESULT_CHECK(hr, L"%s", L"context->GetStatus failed") else {
        DBG_DPRINT(
            L"d=%c%c%c s=%c%c%c",
            (st.dwDynamicFlags & TF_SD_LOADING) ? L'L' : L'_',
            (st.dwDynamicFlags & TF_SD_READONLY) ? L'R' : L'_',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'U' : L'_',
            (st.dwStaticFlags & TF_SS_DISJOINTSEL) ? L'D' : L'_',
            (st.dwStaticFlags & TF_SS_REGIONS) ? L'R' : L'_',
            (st.dwStaticFlags & TF_SS_TRANSITORY) ? L'T' : L'_');
    }
#endif

    Compartment<long> compBackconvert;
    hr = compBackconvert.Initialize(context, compositionManager->GetClientId(), Globals::GUID_Compartment_Backconvert);
    if (SUCCEEDED(hr)) {
        compBackconvert.SetValue(0);
    } else {
        DBG_HRESULT_CHECK(hr, L"%s", L"compBackconvert.Initialize failed");
    }

    hr = SetLangidRequest(context, compositionManager->GetClientId(), true);
    DBG_HRESULT_CHECK(hr, L"%s", L"SetLangidRequest failed");

    hr = CompositionManager::RequestEditSession(EditSessions::EditBlocked, compositionManager, context, controller);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"CompositionManager::RequestEditSession failed");
        controller->SetBlocked(EngineController::BlockedKind::Free);
    }

    return S_OK;
}

HRESULT GetLangidRequest(_In_ ITfContext* pic, _In_ TfClientId clientId, _Out_ BOOL* result) {
    HRESULT hr;

    *result = FALSE;

    Compartment<long> compNeedsSetLangid;
    hr = compNeedsSetLangid.Initialize(pic, clientId, Globals::GUID_Compartment_NeedsSetLangid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compNeedsSetLangid.Initialize failed");

    long val;
    hr = compNeedsSetLangid.GetValue(&val);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compNeedsSetLangid.GetValue failed");

    *result = val == -1;
    return S_OK;
}

HRESULT SetLangidRequest(_In_ ITfContext* pic, _In_ TfClientId clientId, _In_ bool needed) {
    HRESULT hr;

    Compartment<long> compNeedsSetLangid;
    hr = compNeedsSetLangid.Initialize(pic, clientId, Globals::GUID_Compartment_NeedsSetLangid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compNeedsSetLangid.Initialize failed");

    hr = compNeedsSetLangid.SetValue(needed ? -1 : 0);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compNeedsSetLangid.SetValue failed");

    return S_OK;
}

} // namespace VietType
