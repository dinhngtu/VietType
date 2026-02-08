// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Context.h"
#include "VirtualDocument.h"
#include "Compartment.h"

namespace VietType {

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

HRESULT Context::SelectLastWord(
    _In_ TfEditCookie ec,
    _In_ ITfContext* context,
    _In_ int ignore,
    _COM_Outptr_ ITfRange** outRange,
    _Out_ std::wstring* word) {
    HRESULT hr;

    *outRange = nullptr;

    std::array<TF_SELECTION, 2> sel{};
    ULONG fetched;
    hr = context->GetSelection(ec, 0, static_cast<ULONG>(sel.size()), sel.data(), &fetched);
    HRESULT_CHECK_RETURN(hr, L"context->GetSelection failed");
    DPRINT(L"fetched %lu selections", fetched);

    CComPtr<ITfRange> range;
    if (fetched > 1) {
        for (ULONG i = 0; i < fetched; i++) {
            sel[i].range->Release();
        }
        return E_FAIL;
    } else if (fetched == 1) {
        range.Attach(sel[0].range);
        BOOL selEmpty;
        hr = range->IsEmpty(ec, &selEmpty);
        if (FAILED(hr) || !selEmpty) {
            return E_FAIL;
        }
    } else {
        return E_FAIL;
    }

    CComPtr<ITfRange> rangeTest;
    hr = range->Clone(&rangeTest);
    HRESULT_CHECK_RETURN(hr, L"range->Clone failed");

    TF_HALTCOND haltcond{
        nullptr,
        TF_ANCHOR_START, // ignored
        TF_HF_OBJECT,
    };

    LONG shifted;
    hr = rangeTest->ShiftStart(ec, -SWF_MAXCHARS, &shifted, &haltcond);
    HRESULT_CHECK_RETURN(hr, L"rangeTest->ShiftStart failed");
    DPRINT(L"shifted %ld", shifted);

    if (shifted >= 0)
        return E_NOTIMPL;
    shifted = -shifted;

    // find word boundary

    std::array<WCHAR, SWF_MAXCHARS> buf;
    ULONG retrieved;
    hr = rangeTest->GetText(ec, 0, &buf[0], std::min(shifted, SWF_MAXCHARS), &retrieved);
    HRESULT_CHECK_RETURN(hr, L"rangeTest->GetText failed");
    DPRINT(L"retrieved %lu", retrieved);

    if (!retrieved)
        return E_NOTIMPL;

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

    // move range to the word we found
    hr = range->ShiftStart(ec, -wordlen - ignore, &shifted, &haltcond);
    HRESULT_CHECK_RETURN(hr, L"range->ShiftStart failed");

    // now we got the selection containing the text

    *outRange = range;
    (*outRange)->AddRef();
    *word = std::wstring(&buf[static_cast<size_t>(retrieved - wordlen - ignore)], wordlen);
    DPRINT(L"word = %s", word->c_str());
    return S_OK;
}

HRESULT Context::EditLastWord(_In_ TfEditCookie ec, _In_ Context* context, _In_ int ignore, _In_ wchar_t push) {
    CComPtr<ITfRange> range;
    std::wstring word;
    HRESULT hr = SelectLastWord(ec, context->GetContext(), ignore, &range, &word);
    HRESULT_CHECK(hr, "SelectLastWord failed");

    hr = BackconvertWord(ec, context, range, &word, push);
    HRESULT_CHECK_RETURN(hr, "BackconvertWord failed");
    return S_OK;
}

HRESULT Context::KillLastWordInFullContext(
    _In_ TfEditCookie ec, _In_ ITfContext* fullContext, _In_ int ignore, _Out_ std::wstring* word) {
    CComPtr<ITfRange> range;
    HRESULT hr = SelectLastWord(ec, fullContext, ignore, &range, word);
    HRESULT_CHECK_RETURN(hr, "SelectLastWord failed");

    hr = range->SetText(ec, 0, L"", 0);
    HRESULT_CHECK_RETURN(hr, L"range->SetText failed");

    return S_OK;
}

HRESULT Context::BackconvertWord(
    _In_ TfEditCookie ec,
    _In_ Context* context,
    _In_opt_ ITfRange* range,
    _In_ const std::wstring* word,
    _In_ wchar_t push) {
    HRESULT hr;

    hr = context->StartCompositionNow(ec);
    HRESULT_CHECK_RETURN(hr, L"context->StartCompositionNow failed");

    if (range) {
        hr = context->GetComposition()->ShiftStart(ec, range);
        HRESULT_CHECK_RETURN(hr, L"context->GetComposition()->ShiftStart failed");
    }

    auto engine = context->GetEngine();
    engine->Reset();
    engine->Backconvert(*word);

    auto displayText = engine->Peek();
    context->SetCompositionText(ec, displayText.c_str(), static_cast<LONG>(displayText.length()));

    if (!push)
        return S_OK;

    return context->EditNextState(ec, context->GetEngine()->PushChar(push));
}

HRESULT Context::RequestEditLastWord(_In_ int ignore, _In_ wchar_t push) {
    HRESULT hr, hrSession;
    std::wstring word;

    if (GetComposition())
        return E_PENDING;

    // get the full context

    CComPtr<ITfContext> fullContext;
    VirtualDocument::FullContextType contextType;
    hr = VirtualDocument::GetFullContext(_context, GetClientId(), &fullContext, &contextType);
    HRESULT_CHECK(hr, L"VirtualDocument::GetFullContext failed");
    if (FAILED(hr)) {
        // just open a new composition
        hr = RequestEditSessionEx(
            BackconvertWord,
            TF_ES_SYNC | TF_ES_READWRITE,
            &hrSession,
            static_cast<ITfRange*>(nullptr),
            static_cast<const std::wstring*>(&word),
            push);
        HRESULT_CHECK_RETURN(hr, L"RequestEditSessionEx(EditLastWord) failed");
        HRESULT_CHECK_RETURN(hrSession, L"EditLastWord failed");
        return S_OK;
    }

    switch (contextType) {
    case VirtualDocument::FullContextType::Original:
    case VirtualDocument::FullContextType::Chromium:
        // oneshot edit is possible
        hr = RequestEditSessionEx(EditLastWord, TF_ES_SYNC | TF_ES_READWRITE, &hrSession, ignore, push);
        HRESULT_CHECK_RETURN(hr, L"RequestEditSessionEx(EditLastWord) failed");
        HRESULT_CHECK_RETURN(hrSession, L"EditLastWord failed");
    case VirtualDocument::FullContextType::Transitory: {
        do {
            CComPtr<EditSession<ITfContext*, int, std::wstring*>> session;
            hr = CreateInitialize(&session, KillLastWordInFullContext, fullContext.p, ignore, &word);
            if (FAILED(hr)) {
                HRESULT_CHECK(hr, L"CreateInitialize(&session) failed");
                break;
            }

            hr = fullContext->RequestEditSession(GetClientId(), session, TF_ES_SYNC | TF_ES_READWRITE, &hrSession);
            if (FAILED(hr)) {
                HRESULT_CHECK(hr, L"RequestEditSession(KillLastWordInFullContext) failed");
                break;
            }
            if (FAILED(hrSession)) {
                HRESULT_CHECK(hrSession, L"KillLastWordInFullContext failed");
                break;
            }
        } while (0);

        hr = RequestEditSessionEx(
            BackconvertWord,
            TF_ES_SYNC | TF_ES_READWRITE,
            &hrSession,
            static_cast<ITfRange*>(nullptr),
            static_cast<const std::wstring*>(&word),
            push);
        HRESULT_CHECK_RETURN(hr, L"RequestEditSessionEx(EditLastWord) failed");
        HRESULT_CHECK_RETURN(hrSession, L"EditLastWord failed");
    }
    }
    return S_OK;
}

} // namespace VietType
