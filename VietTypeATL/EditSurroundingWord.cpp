// SPDX-License-Identifier: GPL-3.0-only

#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"
#include "VirtualDocument.h"

namespace VietType {
namespace EditSessions {

static const long SWF_MAXCHARS = 9; // "nghiêng" + 1 for the padding + 1 for max ignore

static const std::unordered_set<WCHAR> vietnamesechars_notaz = {
    L'\xe0', L'\xc0',
    L'\xe1', L'\xc1',
    L'\xe2', L'\xc2',
    L'\xe3', L'\xc3',
    L'\xe8', L'\xc8',
    L'\xe9', L'\xc9',
    L'\xea', L'\xca',
    L'\xec', L'\xcc',
    L'\xed', L'\xcd',
    L'\xf2', L'\xd2',
    L'\xf3', L'\xd3',
    L'\xf4', L'\xd4',
    L'\xf5', L'\xd5',
    L'\xf9', L'\xd9',
    L'\xfa', L'\xda',
    L'\xfd', L'\xdd',
    L'\x103', L'\x102',
    L'\x111', L'\x110',
    L'\x129', L'\x128',
    L'\x169', L'\x168',
    L'\x1a1', L'\x1a0',
    L'\x1b0', L'\x1af',
    L'\x1ea1', L'\x1ea0',
    L'\x1ea3', L'\x1ea2',
    L'\x1ea5', L'\x1ea4',
    L'\x1ea7', L'\x1ea6',
    L'\x1ea9', L'\x1ea8',
    L'\x1eab', L'\x1eaa',
    L'\x1ead', L'\x1eac',
    L'\x1eaf', L'\x1eae',
    L'\x1eb1', L'\x1eb0',
    L'\x1eb3', L'\x1eb2',
    L'\x1eb5', L'\x1eb4',
    L'\x1eb7', L'\x1eb6',
    L'\x1eb9', L'\x1eb8',
    L'\x1ebb', L'\x1eba',
    L'\x1ebd', L'\x1ebc',
    L'\x1ebf', L'\x1ebe',
    L'\x1ec1', L'\x1ec0',
    L'\x1ec3', L'\x1ec2',
    L'\x1ec5', L'\x1ec4',
    L'\x1ec7', L'\x1ec6',
    L'\x1ec9', L'\x1ec8',
    L'\x1ecb', L'\x1eca',
    L'\x1ecd', L'\x1ecc',
    L'\x1ecf', L'\x1ece',
    L'\x1ed1', L'\x1ed0',
    L'\x1ed3', L'\x1ed2',
    L'\x1ed5', L'\x1ed4',
    L'\x1ed7', L'\x1ed6',
    L'\x1ed9', L'\x1ed8',
    L'\x1edb', L'\x1eda',
    L'\x1edd', L'\x1edc',
    L'\x1edf', L'\x1ede',
    L'\x1ee1', L'\x1ee0',
    L'\x1ee3', L'\x1ee2',
    L'\x1ee5', L'\x1ee4',
    L'\x1ee7', L'\x1ee6',
    L'\x1ee9', L'\x1ee8',
    L'\x1eeb', L'\x1eea',
    L'\x1eed', L'\x1eec',
    L'\x1eef', L'\x1eee',
    L'\x1ef1', L'\x1ef0',
    L'\x1ef3', L'\x1ef2',
    L'\x1ef5', L'\x1ef4',
    L'\x1ef7', L'\x1ef6',
    L'\x1ef9', L'\x1ef8',
};

static bool IsVietnameseCharacter(WCHAR c) {
    if (c >= L'a' && c <= L'z') {
        return true;
    } else if (c >= L'A' && c <= L'Z') {
        return true;
    } else {
        auto tcit = vietnamesechars_notaz.find(c);
        return tcit != vietnamesechars_notaz.end();
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

HRESULT EditSurroundingWord(
    _In_ TfEditCookie ec,
    _In_ CompositionManager* compositionManager,
    _In_ ITfContext* context,
    _In_ EngineController* controller,
    _In_ int ignore) {

    HRESULT hr;

    DBG_DPRINT(L"ec = %ld", ec);

    if (compositionManager->IsComposing()) {
        return S_OK;
    }
    hr = compositionManager->StartCompositionNow(ec, context);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compositionManager->StartComposition failed");

    CComPtr<ITfContext> ppic;
    hr = VirtualDocument::GetVirtualDocumentContext(context, &ppic);
    HRESULT_CHECK_RETURN(hr, L"%s", L"ThreadMgrEventSink::GetTransitoryParentContext failed");

    CComPtr<ITfRange> range;
    hr = compositionManager->GetRange(&range);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_composition->GetRange failed");

    // shift current range backwards

    CComPtr<ITfRange> rangeTest;
    hr = range->Clone(&rangeTest);
    HRESULT_CHECK_RETURN(hr, L"%s", L"range->Clone failed");

    TF_HALTCOND haltcond{
        NULL,
        TF_ANCHOR_START, // ignored
        TF_HF_OBJECT
    };

    LONG shifted;
    hr = rangeTest->ShiftStart(ec, -SWF_MAXCHARS, &shifted, &haltcond);
    HRESULT_CHECK_RETURN(hr, L"%s", L"rangeTest->ShiftStart failed");

    // find word boundary

    std::array<WCHAR, SWF_MAXCHARS> buf;
    ULONG retrieved;
    hr = rangeTest->GetText(ec, 0, &buf[0], shifted, &retrieved);
    HRESULT_CHECK_RETURN(hr, L"%s", L"rangeTest->GetText failed");

    LONG wordlen = 0;
    // start from retrieved - 2 to ignore a character, therefore emulating the backspace
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

#pragma warning(push)
#pragma warning(disable: 26451)
    if (wordlen < 1 || (static_cast<ULONG>(wordlen) < retrieved && !IsSeparatorCharacter(buf.at(retrieved - wordlen - 1L - ignore)))) {
#pragma warning(pop)
        // no word, or word is not bordered by separator character
        hr = compositionManager->EndCompositionNow(ec);
        HRESULT_CHECK_RETURN(hr, L"%s", L"compositionManager->EndCompositionNow failed");
        return S_OK;
    }

    CComPtr<ITfComposition> composition(compositionManager->GetComposition());

    // move composition to the word we found
    hr = range->ShiftStart(ec, -wordlen - ignore, &shifted, &haltcond);
    HRESULT_CHECK_RETURN(hr, L"%s", L"range->ShiftStart failed");
    hr = composition->ShiftStart(ec, range);
    HRESULT_CHECK_RETURN(hr, L"%s", L"composition->ShiftStart failed");

    if (ignore) {
        hr = range->ShiftEnd(ec, -ignore, &shifted, &haltcond);
        HRESULT_CHECK_RETURN(hr, L"%s", L"range->ShiftEnd failed");
        hr = composition->ShiftEnd(ec, range);
        HRESULT_CHECK_RETURN(hr, L"%s", L"composition->ShiftEnd failed");
    }

    hr = compositionManager->SetRangeDisplayAttribute(ec, context, range);
    HRESULT_CHECK_RETURN(hr, L"%s", L"SetRangeDisplayAttribute failed");

    // move selection to end
    hr = compositionManager->MoveCaretToEnd(ec);
    HRESULT_CHECK_RETURN(hr, L"%s", L"compositionManager->MoveCaretToEnd failed");

    // reinitialize engine with text
    controller->GetEngine().Reset();
#pragma warning(push)
#pragma warning(disable: 26451)
    controller->GetEngine().Backconvert(std::wstring(&buf[static_cast<size_t>(retrieved - wordlen - ignore)], wordlen));
#pragma warning(pop)

    if (controller->GetEngine().GetState() != Telex::TelexStates::Valid) {
        // if found a bad word, force-terminate the composition
        //hr = compositionManager->EndCompositionNow(ec);
        //HRESULT_CHECK_RETURN(hr, L"%s", L"compositionManager->EndCompositionNow failed");
        //return S_OK;
    }

    return S_OK;
}

}
}
