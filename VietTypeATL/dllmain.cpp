// SPDX-License-Identifier: GPL-3.0-only

// dllmain.cpp : Implementation of DllMain.

#include "stdafx.h"
#include "resource.h"
#include "VietTypeATL_i.h"
#include "dllmain.h"
#include "Globals.h"

CVietTypeATLModule _AtlModule;

// DLL Entry Point
extern "C" BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved) {
    if (dwReason == DLL_PROCESS_ATTACH) {
        if (!hInstance) {
            return FALSE;
        }
        VietType::Globals::DllInstance = hInstance;
    }
    return _AtlModule.DllMain(dwReason, lpReserved);
}
