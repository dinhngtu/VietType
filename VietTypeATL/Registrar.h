#pragma once

#include "Common.h"

namespace VietType {

MIDL_INTERFACE("05B4B2A2-8AC0-4A11-8F5C-268C4FD4AFAC") IVietTypeRegistrar : public IUnknown {
    STDMETHOD(ActivateProfiles)() = 0;
    STDMETHOD(DeactivateProfiles)() = 0;
    STDMETHOD(IsProfileActivated)() = 0;
};

class ATL_NO_VTABLE CVietTypeRegistrar
    : public CComObjectRootEx<CComSingleThreadModel>,
      public CComCoClass<CVietTypeRegistrar, &VietType::Globals::CLSID_VietTypeRegistrar>,
      public IVietTypeRegistrar {
public:
    CVietTypeRegistrar() = default;
    CVietTypeRegistrar(const CVietTypeRegistrar&) = delete;
    CVietTypeRegistrar& operator=(const CVietTypeRegistrar&) = delete;
    ~CVietTypeRegistrar() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(CVietTypeRegistrar)
    BEGIN_COM_MAP(CVietTypeRegistrar)
    COM_INTERFACE_ENTRY(IVietTypeRegistrar)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via IVietTypeRegistrar
    virtual STDMETHODIMP ActivateProfiles() override;
    virtual STDMETHODIMP DeactivateProfiles() override;
    virtual STDMETHODIMP IsProfileActivated() override;
};
OBJECT_ENTRY_AUTO(VietType::Globals::CLSID_VietTypeRegistrar, CVietTypeRegistrar)

} // namespace VietType
