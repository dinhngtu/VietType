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

static std::vector<GUID> SupportedCategories = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED, // UI-less
    GUID_TFCAT_TIPCAP_COMLESS, // Google says this is required for WoW apps
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, // store apps?
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, // systray on win8+?
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, // display attributes for composition
};

extern "C" __declspec(dllexport) HRESULT __cdecl RegisterProfiles() {
    HRESULT hr;

    if (!VietType::Globals::dllInstance) {
        DBG_DPRINT(L"%s", L"VietType::Globals::dllInstance is invalid");
        return E_UNEXPECTED;
    }

    WCHAR dllPath[MAX_PATH] = { 0 };

    auto dllPathLength = GetModuleFileName(VietType::Globals::dllInstance, dllPath, MAX_PATH);
    if (dllPathLength == 0) {
        WINERROR_RETURN_HRESULT(L"%s", L"GetModuleFileName failed");
    }
    if (dllPathLength >= MAX_PATH) {
        dllPathLength--;
    }
    dllPath[dllPathLength] = 0;
    DBG_DPRINT(L"found text service DLL: %s", dllPath);

    SmartComPtr<ITfInputProcessorProfiles> profiles;
    hr = profiles.CoCreate(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profiles.CoCreate failed");

    hr = profiles->Register(VietType::Globals::CLSID_TextService);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profiles->Register failed");

    hr = profiles->AddLanguageProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        VietType::Globals::TextServiceDescription.c_str(),
        static_cast<LONG>(VietType::Globals::TextServiceDescription.length()),
        dllPath,
        dllPathLength,
        static_cast<ULONG>(-IDI_IMELOGO));
    HRESULT_CHECK_RETURN(hr, L"%s", L"profiles->AddLanguageProfile failed");

    // set language profile display name

    SmartComPtr<ITfInputProcessorProfilesEx> profilesEx;
    hr = profiles->QueryInterface<ITfInputProcessorProfilesEx>(profilesEx.GetAddress());
    HRESULT_CHECK_RETURN(hr, L"%s", L"profiles->QueryInterface failed");

    hr = profilesEx->SetLanguageProfileDisplayName(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        dllPath,
        dllPathLength,
        IDS_VIETTYPE_DESCRIPTION);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profilesEx->SetLanguageProfileDisplayName failed");

    // enable & activate profile

    hr = profiles->EnableLanguageProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        TRUE);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profiles->EnableLanguageProfile failed");

    hr = profiles->ChangeCurrentLanguage(VietType::Globals::TextServiceLangId);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profiles->ChangeCurrentLanguage failed");

    hr = profiles->ActivateLanguageProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile);

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl UnregisterProfiles() {
    HRESULT hr;

    SmartComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreate(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreate failed");

    hr = profileMgr->UnregisterProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        TF_URP_ALLPROFILES);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr->UnregisterProfile failed");

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl RegisterCategories() {
    HRESULT hr;

    SmartComPtr<ITfCategoryMgr> categoryMgr;
    // categoryMgr.CoCreate uses IID_IUnknown then does a QI, while we want to use IID_ITfCategoryMgr directly
    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER, IID_ITfCategoryMgr, reinterpret_cast<void **>(categoryMgr.GetAddress()));
    HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr.CoCreate failed");

    for (auto const& cat : SupportedCategories) {
        DBG_DPRINT(
            L"registering %08lx-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx",
            cat.Data1, cat.Data2, cat.Data3, cat.Data4[0], cat.Data4[1], cat.Data4[2], cat.Data4[3], cat.Data4[4], cat.Data4[5], cat.Data4[6], cat.Data4[7]);
        hr = categoryMgr->RegisterCategory(VietType::Globals::CLSID_TextService, cat, VietType::Globals::CLSID_TextService);
        HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr->RegisterCategory failed");
    }

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl UnregisterCategories() {
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
            HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr->UnregisterCategory failed");
        } else {
            break;
        }
    }

    return S_OK;
}
