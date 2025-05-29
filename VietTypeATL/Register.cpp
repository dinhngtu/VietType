// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Common.h"
#include <Windows.h>
#include <AccCtrl.h>
#include <AclAPI.h>

static constexpr DWORD ILOT_UNINSTALL = 0x00000001;
using InstallLayoutOrTip_t = BOOL(CALLBACK*)(_In_ LPCWSTR psz, DWORD dwFlags);

static const std::array<const GUID*, 6> SupportedCategories = {
    &GUID_TFCAT_TIP_KEYBOARD,
    &GUID_TFCAT_TIPCAP_UIELEMENTENABLED,  // UI-less
    &GUID_TFCAT_TIPCAP_COMLESS,           // Google says this is required for WoW apps
    &GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,  // store apps?
    &GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,    // systray on win8+?
    &GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, // display attributes for composition
};

// allow ALL APPLICATION PACKAGES permissions to query value
static HRESULT SetSettingsKeyAcl() {
    LSTATUS err;

    CRegKey key;
    err = key.Create(HKEY_CURRENT_USER, VietType::Globals::ConfigKeyName, nullptr, 0, READ_CONTROL | WRITE_DAC);
    WINERROR_CHECK_RETURN_HRESULT(err, L"key.Create failed");

    std::vector<BYTE> sdbuf;
    DWORD sdbufsize = SECURITY_DESCRIPTOR_MIN_LENGTH * 2;
    for (int i = 0; i < 2; i++) {
        sdbuf.resize(sdbufsize);
        err = key.GetKeySecurity(DACL_SECURITY_INFORMATION, &sdbuf[0], &sdbufsize);
        if (err != ERROR_INSUFFICIENT_BUFFER) {
            break;
        }
    }
    WINERROR_CHECK_RETURN_HRESULT(err, L"key.GetKeySecurity failed");

    PACL dacl;
    BOOL daclPresent, daclDefaulted;
    if (!GetSecurityDescriptorDacl(&sdbuf[0], &daclPresent, &dacl, &daclDefaulted)) {
        WINERROR_GLE_RETURN_HRESULT(L"GetSecurityDescriptorDacl failed");
    }

    std::array<BYTE, SECURITY_MAX_SID_SIZE> sid;
    DWORD cbSid = static_cast<DWORD>(sid.size());
    if (!CreateWellKnownSid(WinBuiltinAnyPackageSid, NULL, &sid[0], &cbSid)) {
        WINERROR_GLE_RETURN_HRESULT(L"CreateWellKnownSid failed");
    }

    std::array<EXPLICIT_ACCESS, 2> ea{};

    ea[0].grfAccessPermissions = KEY_ALL_ACCESS;
    ea[0].grfAccessMode = REVOKE_ACCESS;
    ea[0].grfInheritance = NO_INHERITANCE;
    ea[0].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[0].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[0].Trustee.ptstrName = reinterpret_cast<LPWCH>(&sid[0]);

    ea[1].grfAccessPermissions = KEY_QUERY_VALUE;
    ea[1].grfAccessMode = GRANT_ACCESS;
    ea[1].grfInheritance = NO_INHERITANCE;
    ea[1].Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ea[1].Trustee.TrusteeType = TRUSTEE_IS_WELL_KNOWN_GROUP;
    ea[1].Trustee.ptstrName = reinterpret_cast<LPWCH>(&sid[0]);

    PACL pNewAcl;
    err = SetEntriesInAcl(static_cast<ULONG>(ea.size()), ea.data(), dacl, &pNewAcl);
    WINERROR_CHECK_RETURN_HRESULT(err, L"SetEntriesInAcl failed");
    std::unique_ptr<ACL, decltype(&LocalFree)> newAcl(pNewAcl, &LocalFree);

    std::vector<BYTE> newSdBuf(SECURITY_DESCRIPTOR_MIN_LENGTH);
    if (!InitializeSecurityDescriptor(&newSdBuf[0], SECURITY_DESCRIPTOR_REVISION)) {
        WINERROR_GLE_RETURN_HRESULT(L"InitializeSecurityDescriptor failed");
    }

    if (!SetSecurityDescriptorDacl(&newSdBuf[0], TRUE, newAcl.get(), daclDefaulted)) {
        WINERROR_GLE_RETURN_HRESULT(L"SetSecurityDescriptorDacl failed");
    }

    err = key.SetKeySecurity(DACL_SECURITY_INFORMATION, &newSdBuf[0]);
    WINERROR_CHECK_RETURN_HRESULT(err, L"key.SetKeySecurity failed");

    return S_OK;
}

static HRESULT RegisterProfiles() {
    HRESULT hr;

    if (!VietType::Globals::DllInstance) {
        DBG_DPRINT(L"VietType::Globals::DllInstance is invalid");
        return E_UNEXPECTED;
    }

    WCHAR dllPath[MAX_PATH] = {0};

    auto dllPathLength = GetModuleFileName(VietType::Globals::DllInstance, dllPath, MAX_PATH);
    if (dllPathLength == 0) {
        WINERROR_GLE_RETURN_HRESULT(L"GetModuleFileName failed");
    }
    if (dllPathLength >= MAX_PATH) {
        dllPathLength--;
    }
    dllPath[dllPathLength] = 0;
    DBG_DPRINT(L"found text service DLL: %s", dllPath);

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->RegisterProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        VietType::Globals::TextServiceDescription,
        static_cast<ULONG>(wcslen(VietType::Globals::TextServiceDescription)),
        dllPath,
        dllPathLength,
        static_cast<ULONG>(-IDI_IMELOGO),
        NULL,
        0,
        FALSE,
        0);
    HRESULT_CHECK_RETURN(hr, L"profileMgr->RegisterProfile failed");

    return S_OK;
}

static HRESULT UnregisterProfiles() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->UnregisterProfile(
        VietType::Globals::CLSID_TextService,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::GUID_Profile,
        TF_URP_ALLPROFILES);

    return S_OK;
}

static HRESULT RegisterCategories() {
    HRESULT hr;

    CComPtr<ITfCategoryMgr> categoryMgr;
    hr = categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"categoryMgr.CoCreateInstance failed");

    for (const auto cat : SupportedCategories) {
        DBG_DPRINT(L"registering " GUID_WFORMAT, GUID_COMPONENTS(*cat));
        hr = categoryMgr->RegisterCategory(
            VietType::Globals::CLSID_TextService, *cat, VietType::Globals::CLSID_TextService);
        HRESULT_CHECK_RETURN(hr, L"categoryMgr->RegisterCategory failed");
    }

    return S_OK;
}

static HRESULT UnregisterCategories() {
    HRESULT hr;

    CComPtr<ITfCategoryMgr> categoryMgr;
    hr = categoryMgr.CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"categoryMgr.CoCreateInstance failed");

    CComPtr<IEnumGUID> registeredCategories;
    hr = categoryMgr->EnumCategoriesInItem(VietType::Globals::CLSID_TextService, &registeredCategories);
    HRESULT_CHECK_RETURN(hr, L"categoryMgr->EnumCategoriesInItem failed");

    GUID cat = {0};
    ULONG fetched = 0;
    while (1) {
        hr = registeredCategories->Next(1, &cat, &fetched);
        if (hr == S_OK) {
            DBG_DPRINT(L"unregistering " GUID_WFORMAT, GUID_COMPONENTS(cat));
            hr = categoryMgr->UnregisterCategory(
                VietType::Globals::CLSID_TextService, cat, VietType::Globals::CLSID_TextService);
            HRESULT_CHECK_RETURN(hr, L"categoryMgr->UnregisterCategory failed");
        } else {
            break;
        }
    }

    return S_OK;
}

STDAPI DllUnregisterServer() {
    UnregisterCategories();
    UnregisterProfiles();
    return S_OK;
}

STDAPI DllRegisterServer() {
    HRESULT hr;
    hr = RegisterProfiles();
    if (FAILED(hr))
        goto fail;
    hr = RegisterCategories();
    if (FAILED(hr))
        goto fail;
    return S_OK;
fail:
    DllUnregisterServer();
    return hr;
}

static std::wstring GetTipString() {
    HRESULT hr;
    std::array<wchar_t, 4 + 1 + 2 * 38 + 1> tipString = {0};
    hr = StringCchPrintfW(
        tipString.data(), tipString.size(), L"%04X:{%036X}{%036X}", VietType::Globals::TextServiceLangId, 0, 0);
    if (FAILED(hr))
        return std::wstring();
    hr = StringFromGUID2(VietType::Globals::CLSID_TextService, &tipString[4 + 1], 39) ? S_OK : E_FAIL;
    if (FAILED(hr))
        return std::wstring();
    hr = StringFromGUID2(VietType::Globals::GUID_Profile, &tipString[4 + 1 + 38], 39) ? S_OK : E_FAIL;
    if (FAILED(hr))
        return std::wstring();
    *tipString.rbegin() = 0;
    return std::wstring(tipString.data());
}

static HRESULT InstallTip(bool install) {
    HRESULT hr;
    HMODULE inputDll = NULL;
    InstallLayoutOrTip_t ilot = NULL;

    auto tipString = GetTipString();
    if (tipString.empty()) {
        hr = E_FAIL;
        goto out;
    }

    inputDll = LoadLibraryW(L"input.dll");
    if (!inputDll) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto out;
    }

    ilot = reinterpret_cast<InstallLayoutOrTip_t>(GetProcAddress(inputDll, "InstallLayoutOrTip"));
    if (!ilot) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto out_module;
    }

    hr = ilot(tipString.c_str(), install ? 0 : ILOT_UNINSTALL) ? S_OK : E_FAIL;

out_module:
    FreeLibrary(inputDll);
out:
    return hr;
}

extern "C" HRESULT __cdecl ActivateProfiles() {
    HRESULT hr;
    LSTATUS err;

    {
        CRegKey key;
        err = key.Create(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes", nullptr, 0, KEY_SET_VALUE);
        WINERROR_CHECK_RETURN_HRESULT(err, L"key.Create(Keyboard Layout\\Substitutes) failed");

        err = key.SetStringValue(L"0000042a", L"00000409");
        WINERROR_CHECK_RETURN_HRESULT(err, L"key.SetStringValue failed");
    }

    hr = SetSettingsKeyAcl();
    HRESULT_CHECK_RETURN(hr, L"SetSettingsKeyAcl failed");

    hr = InstallTip(true);
    HRESULT_CHECK_RETURN(hr, L"InstallTip failed");

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"profileMgr.CoCreateInstance failed");

    hr = profileMgr->ActivateProfile(
        TF_PROFILETYPE_INPUTPROCESSOR,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::CLSID_TextService,
        VietType::Globals::GUID_Profile,
        NULL,
        TF_IPPMF_FORPROCESS | TF_IPPMF_FORSESSION);

    return S_OK;
}

extern "C" HRESULT __cdecl DeactivateProfiles() {
    HRESULT hr;
    LSTATUS err;

    hr = InstallTip(false);
    HRESULT_CHECK_RETURN(hr, L"InstallTip failed");

    {
        CRegKey key;
        err = key.Create(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes", nullptr, 0, KEY_SET_VALUE);
        WINERROR_CHECK_RETURN_HRESULT(err, L"key.Create(Keyboard Layout\\Substitutes) failed");
        key.DeleteValue(L"0000042a");
    }

    return S_OK;
}

extern "C" HRESULT __cdecl IsProfileActivated() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"profileMgr.CoCreateInstance failed");

    TF_INPUTPROCESSORPROFILE profile;
    hr = profileMgr->GetProfile(
        TF_PROFILETYPE_INPUTPROCESSOR,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::CLSID_TextService,
        VietType::Globals::GUID_Profile,
        NULL,
        &profile);
    HRESULT_CHECK_RETURN(hr, L"profileMgr->GetProfile failed");

    return (profile.dwFlags & TF_IPP_FLAG_ENABLED) ? S_OK : S_FALSE;
}
