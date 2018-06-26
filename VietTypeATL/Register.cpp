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

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Common.h"
#include <MsiQuery.h>

#define MSI_ACTION_ENTRY_POINT(fun) extern "C" __declspec(dllexport) UINT __stdcall fun ## Msi(MSIHANDLE hInstall) { \
    auto hr = fun(); \
    if (SUCCEEDED(hr)) { \
        return ERROR_SUCCESS; \
    } else { \
        return ERROR_INSTALL_FAILURE; \
    } \
}

static std::vector<GUID> SupportedCategories = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED, // UI-less
    GUID_TFCAT_TIPCAP_COMLESS, // Google says this is required for WoW apps
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, // store apps?
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, // systray on win8+?
};

__declspec(dllexport) HRESULT RegisterProfiles() {
    HRESULT hr;

    if (!VietType::Globals::dllInstance) {
        DPRINT(L"%s", L"VietType::Globals::dllInstance is invalid");
        return E_UNEXPECTED;
    }

    WCHAR dllPath[MAX_PATH] = { 0 };

    auto dllPathLength = GetModuleFileName(VietType::Globals::dllInstance, dllPath, MAX_PATH);
    auto err = GetLastError();
    if (err != ERROR_SUCCESS) {
        WINERROR_PRINT(err, L"%s", L"GetModuleFileName failed");
        return HRESULT_FROM_WIN32(err);
    }
    if (dllPathLength >= MAX_PATH) {
        dllPathLength--;
    }
    dllPath[dllPathLength] = 0;
    DPRINT(L"found text service DLL: %s", dllPath);

    SmartComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreate(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreate failed");

    hr = profileMgr->RegisterProfile(
        VietType::Globals::CLSID_TextService,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        VietType::Globals::GUID_Profile,
        VietType::Globals::TextServiceDescription.c_str(),
        static_cast<LONG>(VietType::Globals::TextServiceDescription.length()),
        dllPath, // icon file path
        dllPathLength, // icon file name path
        -IDI_IMELOGO, // icon index has to be negative for some reason
        NULL, // hklSubstitute
        0,
        TRUE,
        0);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr->RegisterProfile failed");

    return S_OK;
}
MSI_ACTION_ENTRY_POINT(RegisterProfiles);

__declspec(dllexport) HRESULT UnregisterProfiles() {
    HRESULT hr;

    SmartComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreate(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreate failed");

    hr = profileMgr->UnregisterProfile(
        VietType::Globals::CLSID_TextService,
        MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
        VietType::Globals::GUID_Profile,
        TF_URP_ALLPROFILES);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr->UnregisterProfile failed");

    return S_OK;
}
MSI_ACTION_ENTRY_POINT(UnregisterProfiles);

__declspec(dllexport) HRESULT RegisterCategories() {
    HRESULT hr;

    SmartComPtr<ITfCategoryMgr> categoryMgr;
    // categoryMgr.CoCreate uses IID_IUnknown then does a QI, while we want to use IID_ITfCategoryMgr directly
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, reinterpret_cast<void **>(categoryMgr.GetAddress()));
    HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr.CoCreate failed");

    for (auto const& cat : SupportedCategories) {
        DBG_DPRINT(
            L"unregistering %08lx-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
            cat.Data1, cat.Data2, cat.Data3, cat.Data4[0], cat.Data4[1], cat.Data4[2], cat.Data4[3], cat.Data4[4], cat.Data4[5], cat.Data4[6], cat.Data4[7]);
        hr = categoryMgr->RegisterCategory(VietType::Globals::CLSID_TextService, cat, VietType::Globals::CLSID_TextService);
        DBG_HRESULT_CHECK(hr, L"%s", L"categoryMgr->RegisterCategory failed");
    }

    return S_OK;
}
MSI_ACTION_ENTRY_POINT(RegisterCategories);

__declspec(dllexport) HRESULT UnregisterCategories() {
    HRESULT hr;

    SmartComPtr<ITfCategoryMgr> categoryMgr;
    // categoryMgr.CoCreate uses IID_IUnknown then does a QI, while we want to use IID_ITfCategoryMgr directly
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, reinterpret_cast<void **>(categoryMgr.GetAddress()));
    HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr.CoCreate failed");

    SmartComPtr<IEnumGUID> registeredCategories;
    hr = categoryMgr->EnumCategoriesInItem(VietType::Globals::CLSID_TextService, registeredCategories.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr->EnumCategoriesInItem failed");

    GUID cat = { 0 };
    ULONG fetched = 0;
    while (1) {
        hr = registeredCategories->Next(1, &cat, &fetched);
        if (hr == S_OK) {
            DBG_DPRINT(
                L"unregistering %08lx-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
                cat.Data1, cat.Data2, cat.Data3, cat.Data4[0], cat.Data4[1], cat.Data4[2], cat.Data4[3], cat.Data4[4], cat.Data4[5], cat.Data4[6], cat.Data4[7]);
            hr = categoryMgr->UnregisterCategory(VietType::Globals::CLSID_TextService, cat, VietType::Globals::CLSID_TextService);
            DBG_HRESULT_CHECK(hr, L"%s", L"categoryMgr->UnregisterCategory failed");
        } else {
            break;
        }
    }

    return S_OK;
}
MSI_ACTION_ENTRY_POINT(UnregisterCategories);
