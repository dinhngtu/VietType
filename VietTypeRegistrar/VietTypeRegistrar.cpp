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

__declspec(dllimport) HRESULT RegisterProfiles();
__declspec(dllimport) HRESULT UnregisterProfiles();
__declspec(dllimport) HRESULT RegisterCategories();
__declspec(dllimport) HRESULT UnregisterCategories();

// registrar 0|1 0|1
// first argument: register(0)/unregister(1)
// second argument: categories(0)/profiles(1)
// order: install 00-01 / uninstall 11-10
int main(int argc, wchar_t **argv) {
    HRESULT hr;

    if (argc != 3) {
        return E_FAIL;
    }

    int a1;
    if (argv[1][0] == L'0') {
        a1 = 0;
    } else if (argv[1][0] == L'1') {
        a1 = 1;
    } else {
        return E_FAIL;
    }

    int a2;
    if (argv[2][0] == L'0') {
        a2 = 0;
    } else if (argv[2][0] == L'1') {
        a2 = 1;
    } else {
        return E_FAIL;
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        return hr;
    }

    if (a1 == 0 && a2 == 0) {
        wprintf_s(L"RegisterCategories");
        hr = RegisterCategories();
    } else if (a1 == 0 && a2 == 1) {
        wprintf_s(L"RegisterProfiles");
        hr = RegisterProfiles();
    } else if (a1 == 1 && a2 == 1) {
        wprintf_s(L"UnregisterProfiles");
        hr = UnregisterProfiles();
    } else if (a1 == 1 && a2 == 0) {
        wprintf_s(L"UnregisterCategories");
        hr = UnregisterCategories();
    }

    CoUninitialize();

    wprintf_s(L"%lx\n", hr);
    return hr;
}

