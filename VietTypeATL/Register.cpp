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

// Derived from Microsoft's SampleIME source code included in the Windows classic samples:
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Common.h"
#include <Windows.h>
#include <AccCtrl.h>
#include <AclAPI.h>

static std::vector<GUID> SupportedCategories = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED, // UI-less
    GUID_TFCAT_TIPCAP_COMLESS, // Google says this is required for WoW apps
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, // store apps?
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, // systray on win8+?
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, // display attributes for composition
};

// allow ALL APPLICATION PACKAGES permissions to query/set value, otherwise we can't change config from within IME
extern "C" __declspec(dllexport) HRESULT __cdecl SetSettingsKeyAcl() {
    LSTATUS err;

    CRegKey key;
    err = key.Create(HKEY_CURRENT_USER, L"Software\\VietType", nullptr, 0, READ_CONTROL | WRITE_DAC);
    WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.Open failed");

    std::vector<BYTE> sdbuf;
    DWORD sdbufsize = SECURITY_DESCRIPTOR_MIN_LENGTH * 2;
    for (int i = 0; i < 2; i++) {
        sdbuf.resize(sdbufsize);
        err = key.GetKeySecurity(DACL_SECURITY_INFORMATION, &sdbuf[0], &sdbufsize);
        if (err != ERROR_INSUFFICIENT_BUFFER) {
            break;
        }
    }
    WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.GetKeySecurity failed");

    PACL dacl;
    BOOL daclPresent, daclDefaulted;
    if (!GetSecurityDescriptorDacl(&sdbuf[0], &daclPresent, &dacl, &daclDefaulted)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetSecurityDescriptorDacl failed");
    }

    std::array<BYTE, SECURITY_MAX_SID_SIZE> sid;
    DWORD cbSid = static_cast<DWORD>(sid.size());
    if (!CreateWellKnownSid(WinBuiltinAnyPackageSid, NULL, &sid[0], &cbSid)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"CreateWellKnownSid failed");
    }

    EXPLICIT_ACCESS ea;
    ea.grfAccessPermissions = KEY_QUERY_VALUE | KEY_SET_VALUE;
    ea.grfAccessMode = GRANT_ACCESS;
    ea.grfInheritance = NO_INHERITANCE;
    ea.Trustee.pMultipleTrustee = nullptr;
    ea.Trustee.MultipleTrusteeOperation = NO_MULTIPLE_TRUSTEE;
    ea.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea.Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea.Trustee.ptstrName = reinterpret_cast<LPWCH>(&sid[0]);

    PACL pNewAcl;
    err = SetEntriesInAcl(1, &ea, dacl, &pNewAcl);
    WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"SetEntriesInAcl failed");
    std::unique_ptr<ACL, decltype(&LocalFree)> newAcl(pNewAcl, &LocalFree);

    std::vector<BYTE> newSdBuf(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!InitializeSecurityDescriptor(&newSdBuf[0], SECURITY_DESCRIPTOR_REVISION)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"InitializeSecurityDescriptor failed");
    }

    if (!SetSecurityDescriptorDacl(&newSdBuf[0], TRUE, newAcl.get(), daclDefaulted)) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"SetSecurityDescriptorDacl failed");
    }

    err = key.SetKeySecurity(DACL_SECURITY_INFORMATION, &newSdBuf[0]);
    WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.SetKeySecurity failed");

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl RegisterProfiles() {
    HRESULT hr;

    if (!VietType::Globals::DllInstance) {
        DBG_DPRINT(L"%s", L"VietType::Globals::DllInstance is invalid");
        return E_UNEXPECTED;
    }

    WCHAR dllPath[MAX_PATH] = { 0 };

    auto dllPathLength = GetModuleFileName(VietType::Globals::DllInstance, dllPath, MAX_PATH);
    if (dllPathLength == 0) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"GetModuleFileName failed");
    }
    if (dllPathLength >= MAX_PATH) {
        dllPathLength--;
    }
    dllPath[dllPathLength] = 0;
    DBG_DPRINT(L"found text service DLL: %s", dllPath);

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->RegisterProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        VietType::Globals::TextServiceDescription.c_str(),
        static_cast<LONG>(VietType::Globals::TextServiceDescription.length()),
        dllPath,
        dllPathLength,
        static_cast<ULONG>(-IDI_IMELOGO),
        NULL,
        0,
        FALSE,
        0);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr->RegisterProfile failed");

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl UnregisterProfiles() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->UnregisterProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        TF_URP_ALLPROFILES);

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl RegisterCategories() {
    HRESULT hr;

    CComPtr<ITfCategoryMgr> categoryMgr;
    hr = categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr.CoCreateInstance failed");

    for (const auto& cat : SupportedCategories) {
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

    CComPtr<ITfCategoryMgr> categoryMgr;
    hr = categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"categoryMgr.CoCreateInstance failed");

    CComPtr<IEnumGUID> registeredCategories;
    hr = categoryMgr->EnumCategoriesInItem(VietType::Globals::CLSID_TextService, &registeredCategories);
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

extern "C" __declspec(dllexport) HRESULT __cdecl ActivateProfiles() {
    HRESULT hr;

    hr = SetSettingsKeyAcl();
    HRESULT_CHECK_RETURN(hr, L"%s", L"SetSettingsKeyAcl failed");

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->ActivateProfile(
        TF_PROFILETYPE_INPUTPROCESSOR,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::CLSID_TextService,
        VietType::Globals::GUID_Profile,
        NULL,
        TF_IPPMF_ENABLEPROFILE | TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE);

    return S_OK;
}

extern "C" __declspec(dllexport) HRESULT __cdecl DeactivateProfiles() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->DeactivateProfile(
        TF_PROFILETYPE_INPUTPROCESSOR,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::CLSID_TextService,
        VietType::Globals::GUID_Profile,
        NULL,
        TF_IPPMF_DISABLEPROFILE);

    return S_OK;
}
