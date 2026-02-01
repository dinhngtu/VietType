// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Telex.h"
#include "KeyTranslator.h"
#include "KeyHandler.h"
#include "EditSessions.h"
#include "CompositionManager.h"
#include "EngineController.h"

namespace VietType {

// {8CC27CF8-93D2-416C-B1A3-66827F54244A}
const GUID GUID_KeyEventSink_PreservedKey_Toggle = {
    0x8cc27cf8, 0x93d2, 0x416c, {0xb1, 0xa3, 0x66, 0x82, 0x7f, 0x54, 0x24, 0x4a}};

STDMETHODIMP CompositionManager::OnSetFocus(_In_ BOOL fForeground) {
    HRESULT hr;

    DBG_DPRINT(L"foreground %d", fForeground);

    _composition.Release();
    _context.Release();

    if (!fForeground) {
        return S_OK;
    }

    CComPtr<ITfDocumentMgr> docMgr;
    hr = _threadMgr->GetFocus(&docMgr);
    if (FAILED(hr)) {
        // we don't care about the error since there might be no focused document manager
        _controller->SetBlocked(EngineController::BlockedKind::Blocked);
        return S_OK;
    }

    return OnSetFocus(docMgr, NULL);
}

DWORD CompositionManager::OnBackconvertBackspace(
    _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten, _In_ DWORD prevBackconvert) {
    Compartment<long> compBackconvert;
    HRESULT hr;
    bool backconvert = false;

    *pfEaten = FALSE;

    hr = compBackconvert.Initialize(_context, _clientid, Globals::GUID_Compartment_Backconvert);
    if (FAILED(hr)) {
        DBG_HRESULT_CHECK(hr, L"compBackconvert.Initialize failed");
        goto finish;
    }

    if (wParam == VK_BACK && !IsComposing() && !IsModifier(_keyState)) {
        long backspacing;
        hr = compBackconvert.GetValue(&backspacing);
        if (FAILED(hr)) {
            DBG_HRESULT_CHECK(hr, L"compBackconvert.GetValue failed");
            goto finish;
        }

        backconvert = backspacing != -1;
    }

finish:
    if (backconvert) {
        *pfEaten = TRUE;
        return prevBackconvert;
    } else {
        hr = compBackconvert.SetValue(0);
        DBG_HRESULT_CHECK(hr, L"compBackconvert reset failed");
        *pfEaten = IsKeyEaten(&_controller->GetEngine(), IsComposing(), wParam, lParam, _keyState);
        return BackconvertDisabled;
    }
}

DWORD CompositionManager::OnBackconvertRetype(
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _In_ DWORD prevBackconvert,
    _Out_ wchar_t* acceptedChar) {
    if (!IsComposing() && !IsModifier(_keyState) &&
        IsKeyAccepted(&_controller->GetEngine(), wParam, lParam, _keyState, acceptedChar)) {
        *pfEaten = TRUE;
        return prevBackconvert;
    } else {
        *pfEaten = IsKeyEaten(&_controller->GetEngine(), IsComposing(), wParam, lParam, _keyState);
        *acceptedChar = 0;
        return 0;
    }
}

HRESULT CompositionManager::OnKeyDownCommon(
    _In_ WPARAM wParam,
    _In_ LPARAM lParam,
    _Out_ BOOL* pfEaten,
    _Out_ DWORD* isBackconvert,
    _Out_ wchar_t* acceptedChar) {
    *pfEaten = FALSE;
    *isBackconvert = BackconvertDisabled;
    *acceptedChar = 0;

    if (!_controller->IsEnabled()) {
        return S_OK;
    }

    if (!GetKeyboardState(_keyState)) {
        WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
    }

    *isBackconvert = _controller->IsBackconvert();
    switch (*isBackconvert) {
    case BackconvertOnBackspace:
        *isBackconvert = OnBackconvertBackspace(wParam, lParam, pfEaten, *isBackconvert);
        break;
    case BackconvertOnType:
        *isBackconvert = OnBackconvertRetype(wParam, lParam, pfEaten, *isBackconvert, acceptedChar);
        break;
    default:
        *pfEaten = IsKeyEaten(&_controller->GetEngine(), IsComposing(), wParam, lParam, _keyState);
        *isBackconvert = BackconvertDisabled;
        break;
    }

    return S_OK;
}

HRESULT CompositionManager::CallKeyEditBackspace(
    _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hrSession;
    HRESULT hr;

    hr = RequestEditSessionEx(
        EditSessions::EditSurroundingWord, TF_ES_SYNC | TF_ES_READWRITE, &hrSession, _controller.p, 1);

    if (FAILED(hr) || FAILED(hrSession)) {
        DBG_DPRINT(L"backspace SendInput fallback");

        CComPtr<ITfCompartmentMgr> tcMgr;
        hr = _context->QueryInterface(&tcMgr);
        HRESULT_CHECK_RETURN(hr, L"pic->QueryInterface failed");

        INPUT inp[2];
        ZeroMemory(inp, sizeof(inp));
        inp[0].type = inp[1].type = INPUT_KEYBOARD;
        inp[0].ki.wVk = inp[1].ki.wVk = VK_BACK;
        inp[1].ki.dwFlags = KEYEVENTF_KEYUP;
        if (!SendInput(ARRAYSIZE(inp), inp, sizeof(INPUT))) {
            WINERROR_GLE_RETURN_HRESULT(L"SendInput failed");
        }
    }

    return S_OK;
}

HRESULT CompositionManager::CallKeyEditRetype(
    _In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState, _In_ wchar_t push) {
    HRESULT hrSession;
    HRESULT hr;

    hr = RequestEditSessionEx(
        EditSessions::EditSurroundingWordAndPush,
        TF_ES_ASYNCDONTCARE | TF_ES_READWRITE,
        &hrSession,
        _controller.p,
        0,
        push);
    HRESULT_CHECK_RETURN(hr, L"RequestEditSession EditSurroundingWord failed");
    HRESULT_CHECK_RETURN(hrSession, L"EditSurroundingWord hrSession failed");

    return S_OK;
}

STDMETHODIMP CompositionManager::OnTestKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    DWORD isBackconvert;
    wchar_t c;

    if (pic != _context) {
        *pfEaten = FALSE;
        return S_OK;
    }

    HRESULT hr = OnKeyDownCommon(wParam, lParam, pfEaten, &isBackconvert, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyDownCommon failed");

    // break off the composition early at OnTestKeyDown on an uneaten key
    if (!*pfEaten && IsComposing()) {
        hr = CallKeyEdit(wParam, lParam, _keyState);
        HRESULT_CHECK_RETURN(hr, L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP CompositionManager::OnTestKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    if (!_controller->IsEnabled() || pic != _context) {
        *pfEaten = FALSE;
        return S_OK;
    }

    // essentially we eat the KeyUp event if a composition is active; is this the correct behavior?
    if (IsComposing()) {
        if (!GetKeyboardState(_keyState)) {
            WINERROR_GLE_RETURN_HRESULT(L"GetKeyboardState failed");
        }

        *pfEaten = IsKeyEaten(&_controller->GetEngine(), IsComposing(), wParam, lParam, _keyState);
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

STDMETHODIMP CompositionManager::OnKeyDown(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    DWORD isBackconvert;
    wchar_t c;

    if (pic != _context) {
        *pfEaten = FALSE;
        return S_OK;
    }

    HRESULT hr = OnKeyDownCommon(wParam, lParam, pfEaten, &isBackconvert, &c);
    HRESULT_CHECK_RETURN(hr, L"OnKeyDownCommon failed");

    DBG_DPRINT(L"OnKeyDown wParam = %lx %s", wParam, *pfEaten ? L"eaten" : L"not eaten");

    if (*pfEaten || IsComposing()) {
        switch (isBackconvert) {
        case BackconvertOnBackspace:
            hr = CallKeyEditBackspace(wParam, lParam, _keyState);
            break;
        case BackconvertOnType:
            hr = CallKeyEditRetype(wParam, lParam, _keyState, c);
            break;
        default:
            hr = CallKeyEdit(wParam, lParam, _keyState);
            break;
        }
        HRESULT_CHECK_RETURN(hr, L"CallKeyEdit failed");
    }

    return S_OK;
}

STDMETHODIMP CompositionManager::OnKeyUp(
    _In_ ITfContext* pic, _In_ WPARAM wParam, _In_ LPARAM lParam, _Out_ BOOL* pfEaten) {
    return OnTestKeyUp(pic, wParam, lParam, pfEaten);
}

STDMETHODIMP CompositionManager::OnPreservedKey(_In_ ITfContext* pic, _In_ REFGUID rguid, _Out_ BOOL* pfEaten) {
    HRESULT hr;

    if (pic != _context) {
        *pfEaten = FALSE;
        return S_OK;
    }

    if (GUID_KeyEventSink_PreservedKey_Toggle == rguid) {
        *pfEaten = TRUE;
        _controller->GetEngine().Reset();
        hr = EndComposition();
        DBG_HRESULT_CHECK(hr, L"_compositionManager->EndComposition failed");
        hr = _controller->ToggleUserEnabled();
        DBG_HRESULT_CHECK(hr, L"_engine->ToggleEnabled failed");
    } else {
        *pfEaten = FALSE;
    }

    return S_OK;
}

HRESULT CompositionManager::CallKeyEdit(_In_ WPARAM wParam, _In_ LPARAM lParam, _In_reads_(256) const BYTE* keyState) {
    HRESULT hr;

    CComPtr<KeyHandlerEditSession> keyHandlerEditSession;
    hr = CreateInitialize(&keyHandlerEditSession, this, _context, _controller.p, wParam, lParam, keyState);
    HRESULT_CHECK_RETURN(hr, L"CreateInitialize(&keyHandlerEditSession) failed");

    HRESULT hrSession;
    hr = _context->RequestEditSession(
        _clientid, keyHandlerEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hrSession);
    HRESULT_CHECK_RETURN(hr, L"_compositionManager->RequestEditSession failed");
    HRESULT_CHECK_RETURN(hrSession, L"KeyHandlerEditSession failed");

    return S_OK;
}

} // namespace VietType
