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

#pragma once

#include "Common.h"
#include "SinkAdvisor.h"

namespace VietType {

class CompositionManager;
class EngineController;

class TextEditSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfTextEditSink {
public:
    TextEditSink();
    ~TextEditSink();

    DECLARE_NOT_AGGREGATABLE(TextEditSink)
    BEGIN_COM_MAP(TextEditSink)
        COM_INTERFACE_ENTRY(ITfTextEditSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    // Inherited via ITfTextEditSink
    virtual STDMETHODIMP OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord) override;

    HRESULT Initialize(ITfDocumentMgr* documentMgr, CompositionManager* compMgr, EngineController* controller);
    HRESULT Uninitialize();

private:
    CComPtr<CompositionManager> _compMgr;
    CComPtr<EngineController> _controller;
    SinkAdvisor<ITfTextEditSink> _textEditSinkAdvisor;
    CComPtr<ITfContext> _editContext;

private:
    DISALLOW_COPY_AND_ASSIGN(TextEditSink);
};

}
