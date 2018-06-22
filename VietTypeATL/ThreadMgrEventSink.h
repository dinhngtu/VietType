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
#include "TextEditSink.h"

using namespace ATL;

namespace VietType {

class ThreadMgrEventSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfThreadMgrEventSink {
public:
    ThreadMgrEventSink();
    ~ThreadMgrEventSink();

    DECLARE_NOT_AGGREGATABLE(ThreadMgrEventSink)
    BEGIN_COM_MAP(ThreadMgrEventSink)
        COM_INTERFACE_ENTRY(ITfThreadMgrEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    // Inherited via ITfThreadMgrEventSink
    virtual STDMETHODIMP OnInitDocumentMgr(ITfDocumentMgr * pdim) override;
    virtual STDMETHODIMP OnUninitDocumentMgr(ITfDocumentMgr * pdim) override;
    virtual STDMETHODIMP OnSetFocus(ITfDocumentMgr * pdimFocus, ITfDocumentMgr * pdimPrevFocus) override;
    virtual STDMETHODIMP OnPushContext(ITfContext * pic) override;
    virtual STDMETHODIMP OnPopContext(ITfContext * pic) override;

    HRESULT Initialize(ITfThreadMgr *threadMgr, TfClientId tid);
    HRESULT Uninitialize();

private:
    SinkAdvisor<ITfThreadMgrEventSink> _threadMgrEventSinkAdvisor;
    SmartComObjPtr<TextEditSink> _textEditSink;
    SmartComPtr<ITfDocumentMgr> _prevFocusDocumentMgr;

private:
    DISALLOW_COPY_AND_ASSIGN(ThreadMgrEventSink);
};

}
