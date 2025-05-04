// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
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
            L"d=%c%c%c%c%c%c%c%c%c s=%c%c%c%c%c%c%c",
            (st.dwDynamicFlags & TS_SD_READONLY) ? L'R' : L'r',
            (st.dwDynamicFlags & TS_SD_LOADING) ? L'L' : L'l',
            (st.dwDynamicFlags & TS_SD_RESERVED) ? L'?' : L'_',
            (st.dwDynamicFlags & TS_SD_TKBAUTOCORRECTENABLE) ? L'A' : L'a',
            (st.dwDynamicFlags & TS_SD_TKBPREDICTIONENABLE) ? L'P' : L'p',
            (st.dwDynamicFlags & TS_SD_UIINTEGRATIONENABLE) ? L'I' : L'i',
            (st.dwDynamicFlags & TS_SD_INPUTPANEMANUALDISPLAYENABLE) ? L'M' : L'm',
            (st.dwDynamicFlags & TS_SD_EMBEDDEDHANDWRITINGVIEW_ENABLED) ? L'H' : L'h',
            (st.dwDynamicFlags & TS_SD_EMBEDDEDHANDWRITINGVIEW_VISIBLE) ? L'V' : L'v',
            (st.dwStaticFlags & TS_SS_DISJOINTSEL) ? L'D' : L'd',
            (st.dwStaticFlags & TS_SS_REGIONS) ? L'R' : L'r',
            (st.dwStaticFlags & TS_SS_TRANSITORY) ? L'T' : L't',
            (st.dwStaticFlags & TS_SS_NOHIDDENTEXT) ? L'H' : L'h',
            (st.dwStaticFlags & TS_SS_TKBAUTOCORRECTENABLE) ? L'A' : L'a',
            (st.dwStaticFlags & TS_SS_TKBPREDICTIONENABLE) ? L'P' : L'p',
            (st.dwStaticFlags & TS_SS_UWPCONTROL) ? L'U' : L'u');
    }
#endif

    Compartment<long> compBackconvert;
    hr = compBackconvert.Initialize(context, compositionManager->GetClientId(), Globals::GUID_Compartment_Backconvert);
    if (SUCCEEDED(hr)) {
        compBackconvert.SetValue(0);
    } else {
        DBG_HRESULT_CHECK(hr, L"%s", L"compBackconvert.Initialize failed");
    }

    hr = CompositionManager::RequestEditSession(EditSessions::EditBlocked, compositionManager, context, controller);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"%s", L"CompositionManager::RequestEditSession failed");
        controller->SetBlocked(EngineController::BlockedKind::Free);
    }

    return S_OK;
}

} // namespace VietType
