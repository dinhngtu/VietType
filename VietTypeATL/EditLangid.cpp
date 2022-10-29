// SPDX-License-Identifier: GPL-3.0-only

#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "ContextUtilities.h"

namespace VietType {
namespace EditSessions {

HRESULT EditLangid(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _In_reads_(256) const BYTE* keyState,
    _In_ EngineController* controller) {
    HRESULT hr;

    hr = SetLangidRequest(context, compositionManager->GetClientId(), true);
    DBG_HRESULT_CHECK(hr, L"%s", L"SetLangidRequest failed");

    std::array<wchar_t, 5> chars{};
    auto count = ToUnicode(wParam, lParam, keyState, chars.data(), chars.size() - 1, 0);
    if (count < 1) {
        return E_FAIL;
    }
    chars[count] = 0;

    DPRINT(L"EditLangid %s", chars.data());

    hr = compositionManager->StartComposition(context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compositionManager->StartComposition failed");

    hr = compositionManager->SetCompositionText(ec, chars.data(), count);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compositionManager->SetCompositionText failed");

    CComPtr<ITfRange> range;
    hr = compositionManager->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compositionManager->GetRange failed");

    hr = compositionManager->SetRangeLangid(ec, context, range, GetUserDefaultLangID());
    DBG_HRESULT_CHECK(hr, L"%s", L"compositionManager->SetRangeLangid failed");

    hr = compositionManager->EndCompositionNow(ec);
    DBG_HRESULT_CHECK(hr, L"%s", L"compositionManager->EndCompositionNow failed");

    return S_OK;
}

} // namespace EditSessions
} // namespace VietType
