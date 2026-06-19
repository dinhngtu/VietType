// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// TextService.h : Declaration of the TextService

#pragma once

#include "Common.h"

namespace VietType {

namespace Telex {
class ITelexEngine;
}

class EnumDisplayAttributeInfo;
class EngineSettingsController;
class CompartmentNotifier;
class ContextManager;

class ATL_NO_VTABLE TextService : public CComObjectRootEx<CComSingleThreadModel>,
                                  public CComCoClass<TextService, &VietType::Globals::CLSID_TextService>,
                                  public ITfTextInputProcessorEx,
                                  public ITfDisplayAttributeProvider {
public:
    TextService() = default;
    TextService(const TextService&) = delete;
    TextService& operator=(const TextService&) = delete;
    ~TextService() = default;

    DECLARE_NO_REGISTRY()
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
    virtual STDMETHODIMP GetDisplayAttributeInfo(
        __RPC__in REFGUID guid, __RPC__deref_out_opt ITfDisplayAttributeInfo** ppInfo) override;

    HRESULT InitializeDisplayAttributes(_In_ ITfThreadMgr* ptim, _In_ TfClientId tid);
    HRESULT UpdateDisplayAttributes(_COM_Outptr_opt_ ITfDisplayAttributeInfo** ppInfo);

private:
    CComPtr<ITfThreadMgr> _threadMgr;
    CComPtr<VietType::EnumDisplayAttributeInfo> _attributeStore;
    CComPtr<EngineSettingsController> _settings;
    CComPtr<CompartmentNotifier> _systemNotify;
    TfGuidAtom _displayAtom = TF_INVALID_GUIDATOM;
    CComPtr<ContextManager> _contextManager;
};
OBJECT_ENTRY_AUTO(VietType::Globals::CLSID_TextService, TextService)

} // namespace VietType
