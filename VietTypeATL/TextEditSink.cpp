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

#include "TextEditSink.h"

VietType::TextEditSink::TextEditSink() {
}

VietType::TextEditSink::~TextEditSink() {
}

STDMETHODIMP VietType::TextEditSink::OnEndEdit(ITfContext * pic, TfEditCookie ecReadOnly, ITfEditRecord * pEditRecord) {
    if (pEditRecord == nullptr) {
        return E_INVALIDARG;
    }

    return S_OK;
}

HRESULT VietType::TextEditSink::Initialize(ITfDocumentMgr *documentMgr) {
    HRESULT hr;

    // we don't care about unadvise result
    _textEditSinkAdvisor.Unadvise();

    if (!documentMgr) {
        // caller just wanted to clear the previous sink
        return S_OK;
    }

    hr = documentMgr->GetTop(_editContext.GetAddress());
    if (FAILED(hr)) {
        return hr;
    }

    if (!_editContext) {
        // empty document, no sink possible
        return S_OK;
    }

    SmartComPtr<ITfSource> source(_editContext);
    if (!source) {
        return E_NOINTERFACE;
    }

    hr = _textEditSinkAdvisor.Advise(source, this);

    return S_OK;
}

HRESULT VietType::TextEditSink::Uninitialize() {
    HRESULT hr;

    hr = _textEditSinkAdvisor.Unadvise();
    DBG_HRESULT_CHECK(hr, L"%s", L"_textEditSinkAdvisor.Unadvise failed");

    _editContext.Release();

    return S_OK;
}
