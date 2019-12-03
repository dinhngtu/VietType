// Copyright (c) 2018 Dinh Ngoc Tu.
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

// TextService.h : Declaration of the TextService

#pragma once

#include "VietTypeATL_i.h"
#include "Common.h"

#if defined(_WIN32_WCE) && !defined(_CE_DCOM) && !defined(_CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA)
#error "Single-threaded COM objects are not properly supported on Windows CE platform, such as the Windows Mobile platforms that do not include full DCOM support. Define _CE_ALLOW_SINGLE_THREADED_OBJECTS_IN_MTA to force ATL to support creating single-thread COM object's and allow use of it's single-threaded COM object implementations. The threading model in your rgs file was set to 'Free' as that is the only threading model supported in non DCOM Windows CE platforms."
#endif

using namespace ATL;

namespace VietType {

namespace Telex {
class TelexEngine;
}

class ThreadMgrEventSink;
class KeyEventSink;
class CompositionManager;
class EngineController;

class ATL_NO_VTABLE TextService :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<TextService, &VietType::Globals::CLSID_TextService>,
    public ITfTextInputProcessorEx,
    public ITfDisplayAttributeProvider {
public:
    TextService() = default;
    TextService(const TextService&) = delete;
    TextService& operator=(const TextService&) = delete;
    ~TextService() = default;

    DECLARE_REGISTRY_RESOURCEID(IDR_TEXTSERVICE)

    DECLARE_NOT_AGGREGATABLE(TextService)
    BEGIN_COM_MAP(TextService)
        COM_INTERFACE_ENTRY(ITfTextInputProcessor)
        COM_INTERFACE_ENTRY(ITfTextInputProcessorEx)
        COM_INTERFACE_ENTRY(ITfDisplayAttributeProvider)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    // Inherited via ITfTextInputProcessorEx
    virtual STDMETHODIMP Activate(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid) override;
    virtual STDMETHODIMP Deactivate(void) override;
    virtual STDMETHODIMP ActivateEx(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid, _In_ DWORD dwFlags) override;

    // Inherited via ITfDisplayAttributeProvider
    virtual STDMETHODIMP EnumDisplayAttributeInfo(__RPC__deref_out_opt IEnumTfDisplayAttributeInfo** ppEnum) override;
    virtual STDMETHODIMP GetDisplayAttributeInfo(__RPC__in REFGUID guid, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo) override;

private:
    CComPtr<ITfThreadMgr> _threadMgr;
    TfClientId _clientId = TF_CLIENTID_NULL;
    DWORD _activateFlags = 0;

    std::shared_ptr<Telex::TelexEngine> _engine;

    CComPtr<ThreadMgrEventSink> _threadMgrEventSink;
    CComPtr<KeyEventSink> _keyEventSink;

    CComPtr<CompositionManager> _compositionManager;

    CComPtr<EngineController> _engineController;
};
OBJECT_ENTRY_AUTO(VietType::Globals::CLSID_TextService, TextService)

}
