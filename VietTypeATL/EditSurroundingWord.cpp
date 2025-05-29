// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "VirtualDocument.h"
#include "Telex.h"

namespace VietType {
namespace EditSessions {

static const long SWF_MAXCHARS = 9; // "nghiêng" + 1 for the padding + 1 for max ignore

static const std::array<WCHAR, 135> vietnamesechars_notaz = {
    L'\xc0',   L'\xc1',   L'\xc2',   L'\xc3',   L'\xc8',   L'\xc9',   L'\xca',   L'\xcc',   L'\xcd',   L'\xd2',
    L'\xd3',   L'\xd4',   L'\xd5',   L'\xd9',   L'\xda',   L'\xdd',   L'\xe0',   L'\xe1',   L'\xe2',   L'\xe3',
    L'\xe8',   L'\xe9',   L'\xea',   L'\xec',   L'\xed',   L'\xf2',   L'\xf3',   L'\xf4',   L'\xf5',   L'\xf9',
    L'\xfa',   L'\xfd',   L'\x102',  L'\x103',  L'\x110',  L'\x111',  L'\x128',  L'\x129',  L'\x168',  L'\x169',
    L'\x1a0',  L'\x1a1',  L'\x1af',  L'\x1b0',  L'\x1ea0', L'\x1ea1', L'\x1ea2', L'\x1ea3', L'\x1ea4', L'\x1ea5',
    L'\x1ea6', L'\x1ea7', L'\x1ea8', L'\x1ea9', L'\x1eaa', L'\x1eab', L'\x1eac', L'\x1ead', L'\x1eae', L'\x1eaf',
    L'\x1eb0', L'\x1eb1', L'\x1eb2', L'\x1eb3', L'\x1eb4', L'\x1eb5', L'\x1eb6', L'\x1eb7', L'\x1eb8', L'\x1eb9',
    L'\x1eba', L'\x1ebb', L'\x1ebc', L'\x1ebd', L'\x1ebe', L'\x1ebf', L'\x1ec0', L'\x1ec1', L'\x1ec2', L'\x1ec3',
    L'\x1ec4', L'\x1ec5', L'\x1ec6', L'\x1ec7', L'\x1ec8', L'\x1ec9', L'\x1eca', L'\x1ecb', L'\x1ecc', L'\x1ecd',
    L'\x1ece', L'\x1ecf', L'\x1ed0', L'\x1ed1', L'\x1ed2', L'\x1ed3', L'\x1ed4', L'\x1ed5', L'\x1ed6', L'\x1ed7',
    L'\x1ed8', L'\x1ed9', L'\x1eda', L'\x1edb', L'\x1edc', L'\x1edd', L'\x1ede', L'\x1edf', L'\x1ee0', L'\x1ee1',
    L'\x1ee2', L'\x1ee3', L'\x1ee4', L'\x1ee5', L'\x1ee6', L'\x1ee7', L'\x1ee8', L'\x1ee9', L'\x1eea', L'\x1eeb',
    L'\x1eec', L'\x1eed', L'\x1eee', L'\x1eef', L'\x1ef0', L'\x1ef1', L'\x1ef2', L'\x1ef3', L'\x1ef4', L'\x1ef5',
    L'\x1ef6', L'\x1ef7', L'\x1ef8', L'\x1ef9', WCHAR_MAX,
};

static bool IsVietnameseCharacter(WCHAR c) {
    if (c >= L'a' && c <= L'z') {
        return true;
    } else if (c >= L'A' && c <= L'Z') {
        return true;
    } else {
        // std::lower_bound should never fail here thanks to WCHAR_MAX
        return c == *std::lower_bound(vietnamesechars_notaz.begin(), vietnamesechars_notaz.end(), c);
    }
}

static bool IsSeparatorCharacter(WCHAR c) {
    if (c >= L' ' && c <= L'@') {
        return true;
    } else if (c >= L'[' && c <= L'`') {
        return true;
    } else if (c >= L'{' && c <= L'~') {
        return true;
    } else if (c == L'\t' || c == L'\n' || c == L'\r') {
        return true;
    }
    return false;
}

static HRESULT DoEditSurroundingWord(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller,
    _In_ int ignore) {
    HRESULT hr;

    CComPtr<ITfContext> ppic;
    hr = VirtualDocument::GetVirtualDocumentContext(context, &ppic);
    DBG_HRESULT_CHECK(hr, L"ThreadMgrEventSink::GetTransitoryParentContext failed");

    if (!ppic) {
        ppic = context;
    }

    CComPtr<ITfRange> range;
    hr = compositionManager->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"_composition->GetRange failed");

    // shift current range backwards

    CComPtr<ITfRange> rangeTest;
    hr = range->Clone(&rangeTest);
    HRESULT_CHECK_RETURN(hr, L"range->Clone failed");

    TF_HALTCOND haltcond{
        NULL,
        TF_ANCHOR_START, // ignored
        TF_HF_OBJECT};

    LONG shifted;
    hr = rangeTest->ShiftStart(ec, -SWF_MAXCHARS, &shifted, &haltcond);
    HRESULT_CHECK_RETURN(hr, L"rangeTest->ShiftStart failed");

    if (shifted >= 0) {
        return E_NOTIMPL;
    }
    shifted = -shifted;

    // find word boundary

    std::array<WCHAR, SWF_MAXCHARS> buf;
    ULONG retrieved;
    hr = rangeTest->GetText(ec, 0, &buf[0], std::min(shifted, SWF_MAXCHARS), &retrieved);
    HRESULT_CHECK_RETURN(hr, L"rangeTest->GetText failed");

    if (!retrieved) {
        return E_NOTIMPL;
    }

    LONG wordlen = 0;
    for (int i = retrieved - 1 - ignore; i >= 0; i--) {
        if (IsVietnameseCharacter(buf[i])) {
            // allowed char, can continue
            wordlen++;
            continue;
        } else {
            break;
        }
    }

    DBG_DPRINT(L"wordlen = %ld", wordlen);

    if (wordlen < 1) {
        return E_FAIL;
    }
    if (retrieved == SWF_MAXCHARS && std::cmp_equal(wordlen, retrieved - ignore) && !IsSeparatorCharacter(buf[0])) {
        return E_FAIL;
    }
    if (std::cmp_less(wordlen + 1, retrieved) && !IsSeparatorCharacter(buf.at(retrieved - wordlen - 1L - ignore))) {
        // word is not bordered by separator character
        return E_FAIL;
    }

    CComPtr<ITfComposition> composition(compositionManager->GetComposition());

    // move composition to the word we found
    hr = range->ShiftStart(ec, -wordlen - ignore, &shifted, &haltcond);
    HRESULT_CHECK_RETURN(hr, L"range->ShiftStart failed");
    hr = composition->ShiftStart(ec, range);
    HRESULT_CHECK_RETURN(hr, L"composition->ShiftStart failed");

    /*
    if (ignore) {
        hr = range->ShiftEnd(ec, -ignore, &shifted, &haltcond);
        HRESULT_CHECK_RETURN(hr, L"range->ShiftEnd failed");
        hr = composition->ShiftEnd(ec, range);
        HRESULT_CHECK_RETURN(hr, L"composition->ShiftEnd failed");
    }
    */

    // reinitialize engine with text
    controller->GetEngine().Reset();
#pragma warning(push)
#pragma warning(disable : 26451)
    controller->GetEngine().Backconvert(std::wstring(&buf[static_cast<size_t>(retrieved - wordlen - ignore)], wordlen));
#pragma warning(pop)

    auto displayText = controller->GetEngine().Peek();
    compositionManager->SetCompositionText(ec, displayText.c_str(), static_cast<LONG>(displayText.length()));

    if (controller->GetEngine().GetState() == Telex::TelexStates::TxError) {
        controller->GetEngine().Reset();
        return E_FAIL;
    }

    return S_OK;
}

HRESULT EditSurroundingWord(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller,
    _In_ int ignore) {

    HRESULT hr;

    DBG_DPRINT(L"backconvert ec = %ld", ec);

    Compartment<long> compBackconvert;
    hr = compBackconvert.Initialize(context, compositionManager->GetClientId(), Globals::GUID_Compartment_Backconvert);
    HRESULT_CHECK_RETURN(hr, L"compBackconvert.Initialize failed");
    hr = compBackconvert.SetValue(-1);
    HRESULT_CHECK_RETURN(hr, L"compBackconvert.SetValue failed");

    if (compositionManager->IsComposing()) {
        return E_PENDING;
    }

    std::array<TF_SELECTION, 2> sel{};
    ULONG fetched;
    hr = context->GetSelection(ec, 0, static_cast<ULONG>(sel.size()), sel.data(), &fetched);
    HRESULT_CHECK_RETURN(hr, L"context->GetSelection failed");

    if (fetched > 1) {
        for (ULONG i = 0; i < fetched; i++) {
            sel[i].range->Release();
        }
        return E_FAIL;
    } else if (fetched == 1) {
        CComPtr<ITfRange> selRange;
        selRange.Attach(sel[0].range);
        BOOL selEmpty;
        hr = selRange->IsEmpty(ec, &selEmpty);
        if (FAILED(hr) || !selEmpty) {
            return E_FAIL;
        }
    }

    hr = compositionManager->StartCompositionNow(ec, context);
    HRESULT_CHECK_RETURN(hr, L"_compositionManager->StartComposition failed");

    hr = DoEditSurroundingWord(ec, compositionManager, context, controller, ignore);
    if (SUCCEEDED(hr)) {
        compBackconvert.SetValue(0);
    } else {
        DBG_HRESULT_CHECK(hr, L"DoEditSurroundingWord failed");
        compositionManager->EndCompositionNow(ec);
        DBG_HRESULT_CHECK(hr, L"compositionManager->EndCompositionNow failed");
    }

    return hr;
}

} // namespace EditSessions
} // namespace VietType
