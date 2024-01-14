// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "TextEditSink.h"
#include "CompositionManager.h"
#include "EngineController.h"

namespace VietType {

STDMETHODIMP TextEditSink::OnEndEdit(
    __RPC__in_opt ITfContext* pic, _In_ TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord* pEditRecord) {
    DBG_DPRINT(L"%s", L"");

    return S_OK;
}

_Check_return_ HRESULT TextEditSink::Initialize(
    _In_ ITfDocumentMgr* documentMgr, _In_ CompositionManager* compMgr, _In_ EngineController* controller) {
    HRESULT hr;

    _compMgr = compMgr;
    _controller = controller;

    hr = _textEditSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_textEditSinkAdvisor.Unadvise failed");

    if (!documentMgr) {
        // caller just wanted to clear the previous sink
        return S_OK;
    }

    hr = documentMgr->GetTop(&_editContext);
    HRESULT_CHECK_RETURN(hr, L"%s", L"documentMgr->GetTop failed");

    if (!_editContext) {
        // empty document, no sink possible
        return S_OK;
    }

    CComPtr<ITfSource> source;
    hr = _editContext->QueryInterface(&source);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_editContext->QueryInterface failed");

    hr = _textEditSinkAdvisor.Advise(source, this);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_textEditSinkAdvisor.Advise failed");

    return S_OK;
}

HRESULT TextEditSink::Uninitialize() {
    HRESULT hr;

    hr = _textEditSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_textEditSinkAdvisor.Unadvise failed");

    _editContext.Release();

    _controller.Release();
    _compMgr.Release();

    return S_OK;
}

} // namespace VietType
