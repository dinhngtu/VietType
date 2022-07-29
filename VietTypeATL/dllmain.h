// SPDX-License-Identifier: GPL-3.0-only

// dllmain.h : Declaration of module class.

class CVietTypeATLModule : public ATL::CAtlDllModuleT<CVietTypeATLModule> {
public:
    DECLARE_LIBID(LIBID_VietTypeATLLib)
    DECLARE_REGISTRY_APPID_RESOURCEID(IDR_VIETTYPEATL, "{4914ef26-8318-4608-aa14-4f7cbb6229e5}")
};

extern class CVietTypeATLModule _AtlModule;
