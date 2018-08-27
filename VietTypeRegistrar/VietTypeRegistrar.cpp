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

// VietTypeRegistrar.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

extern "C" typedef HRESULT(__cdecl* funtype)();

// registrar 0|1 0|1|2
// first argument: register(0)/unregister(1)
// second argument: categories(0)/profiles(1)/activate(2)
// order: install 00-01(-02) / uninstall (12-)11-10
int main(int argc, wchar_t** argv) {
    // only load from exe directory
    SetDllDirectory(L"");

    long result;

    if (argc != 3) {
        wprintf_s(L"usage: registrar 0|1 0|1|2\n");
        return E_FAIL;
    }

    int a1;
    if (argv[1][0] == L'0') {
        a1 = 0;
    } else if (argv[1][0] == L'1') {
        a1 = 1;
    } else if (argv[1][0] == L'2') {
        a1 = 2;
    } else {
        wprintf_s(L"bad parameters\n");
        return E_FAIL;
    }

    int a2;
    if (argv[2][0] == L'0') {
        a2 = 0;
    } else if (argv[2][0] == L'1') {
        a2 = 1;
    } else if (argv[2][0] == L'2') {
        a2 = 2;
    } else {
        wprintf_s(L"bad parameters\n");
        return E_FAIL;
    }

    result = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(result)) {
        wprintf_s(L"CoInitializeEx failed error %lx\n", result);
        return result;
    }

#if _WIN64
    HMODULE mod = LoadLibraryEx(L"VietTypeATL64.dll", NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#elif _WIN32
    HMODULE mod = LoadLibraryEx(L"VietTypeATL32.dll", NULL, LOAD_LIBRARY_SEARCH_DEFAULT_DIRS);
#else
#error "Unsupported architecture"
#endif

    if (!mod) {
        result = GetLastError();
        wprintf_s(L"LoadLibraryEx failed error %lx\n", result);
        goto cleanup;
    }

    funtype fun;

    if (a1 == 0 && a2 == 0) {
        wprintf_s(L"RegisterCategories ");
        fun = reinterpret_cast<funtype>(GetProcAddress(mod, "RegisterCategories"));
    } else if (a1 == 0 && a2 == 1) {
        wprintf_s(L"RegisterProfiles ");
        fun = reinterpret_cast<funtype>(GetProcAddress(mod, "RegisterProfiles"));
    } else if (a1 == 0 && a2 == 2) {
        wprintf_s(L"ActivateProfiles ");
        fun = reinterpret_cast<funtype>(GetProcAddress(mod, "ActivateProfiles"));
    } else if (a1 == 1 && a2 == 2) {
        wprintf_s(L"DeactivateProfiles ");
        fun = reinterpret_cast<funtype>(GetProcAddress(mod, "DeactivateProfiles"));
    } else if (a1 == 1 && a2 == 1) {
        wprintf_s(L"UnregisterProfiles ");
        fun = reinterpret_cast<funtype>(GetProcAddress(mod, "UnregisterProfiles"));
    } else if (a1 == 1 && a2 == 0) {
        wprintf_s(L"UnregisterCategories ");
        fun = reinterpret_cast<funtype>(GetProcAddress(mod, "UnregisterCategories"));
    } else {
        wprintf_s(L"bad parameters\n");
        result = 1;
        goto cleanup;
    }

    if (fun) {
        result = fun();
        wprintf_s(L"%lx\n", result);
    } else {
        result = GetLastError();
        wprintf_s(L"GetProcAddress failed error %lx\n", result);
        goto cleanup;
    }

cleanup:
    if (mod) {
        FreeLibrary(mod);
    }
    CoUninitialize();

    return result;
}

