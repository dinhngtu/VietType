// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"

namespace VietType {

class CompositionManager;
class EngineController;

class TextEditSink : public CComObjectRootEx<CComSingleThreadModel>, public ITfTextEditSink {
public:
    TextEditSink() = default;
    TextEditSink(const TextEditSink&) = delete;
    TextEditSink& operator=(const TextEditSink&) = delete;
    ~TextEditSink() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(TextEditSink)
    BEGIN_COM_MAP(TextEditSink)
    COM_INTERFACE_ENTRY(ITfTextEditSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    // Inherited via ITfTextEditSink
    virtual STDMETHODIMP OnEndEdit(
        __RPC__in_opt ITfContext* pic, _In_ TfEditCookie ecReadOnly, __RPC__in_opt ITfEditRecord* pEditRecord) override;

    _Check_return_ HRESULT
    Initialize(_In_ ITfDocumentMgr* documentMgr, _In_ CompositionManager* compMgr, _In_ EngineController* controller);
    HRESULT Uninitialize();

private:
    CComPtr<CompositionManager> _compMgr;
    CComPtr<EngineController> _controller;
    SinkAdvisor<ITfTextEditSink> _textEditSinkAdvisor;
    CComPtr<ITfContext> _editContext;
};

} // namespace VietType
