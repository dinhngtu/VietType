// SPDX-License-Identifier: GPL-3.0-only

// Derived from Microsoft's SampleIME source code included in the Windows classic samples,
// whose original copyright and permission notice is included below:
//
//     THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
//     ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
//     THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
//     PARTICULAR PURPOSE.
//
//     Copyright (c) Microsoft Corporation. All rights reserved

#include "Common.h"
#include <Windows.h>
#include <AccCtrl.h>
#include <AclAPI.h>

extern "C" typedef HRESULT(__cdecl* funtype)();

static void ShowRunError(HWND hWnd, HRESULT hr) {
    std::wstringstream errmsg;
    errmsg << L"VietType registration error: " << hr;
    MessageBoxW(hWnd, errmsg.str().c_str(), L"VietType", MB_ICONERROR | MB_OK);
}

static void DoRunFunction(funtype fun, HWND hWnd, HINSTANCE hInst, LPWSTR lpszCmdLine, int nCmdShow) {
    HRESULT hr;
    if (!VietType::Globals::DllInstance) {
        VietType::Globals::DllInstance = hInst;
    }
    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        ShowRunError(hWnd, hr);
    } else {
        hr = fun();
        if (FAILED(hr)) {
            ShowRunError(hWnd, hr);
        }
        CoUninitialize();
    }
}

// allow ALL APPLICATION PACKAGES permissions to query value
static HRESULT SetSettingsKeyAcl() {
    LSTATUS err;

    CRegKey key;
    err = key.Create(HKEY_CURRENT_USER, VietType::Globals::ConfigKeyName.c_str(), nullptr, 0, READ_CONTROL | WRITE_DAC);
    WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.Create failed");

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

extern "C" __declspec(dllexport) void CALLBACK
    RunSetSettingsKeyAclW(HWND hWnd, HINSTANCE hInst, LPWSTR lpszCmdLine, int nCmdShow) {
    DoRunFunction(SetSettingsKeyAcl, hWnd, hInst, lpszCmdLine, nCmdShow);
}

extern "C" __declspec(dllexport) HRESULT __cdecl ActivateProfiles() {
    HRESULT hr;
    LSTATUS err;

    {
        CRegKey key;
        err = key.Create(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes", nullptr, 0, KEY_SET_VALUE);
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.Create(Keyboard Layout\\Substitutes) failed");

        err = key.SetStringValue(L"0000042a", L"00000409");
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.SetStringValue failed");
    }

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

extern "C" __declspec(dllexport) void CALLBACK
    RunActivateProfilesW(HWND hWnd, HINSTANCE hInst, LPWSTR lpszCmdLine, int nCmdShow) {
    DoRunFunction(ActivateProfiles, hWnd, hInst, lpszCmdLine, nCmdShow);
}

extern "C" __declspec(dllexport) HRESULT __cdecl DeactivateProfiles() {
    HRESULT hr;
    LSTATUS err;

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

    {
        CRegKey key;
        err = key.Create(HKEY_CURRENT_USER, L"Keyboard Layout\\Substitutes", nullptr, 0, KEY_SET_VALUE);
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.Create(Keyboard Layout\\Substitutes) failed");

        err = key.DeleteValue(L"0000042a");
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"key.SetStringValue failed");
    }

    return S_OK;
}

extern "C" __declspec(dllexport) void CALLBACK
    RunDeactivateProfilesW(HWND hWnd, HINSTANCE hInst, LPWSTR lpszCmdLine, int nCmdShow) {
    DoRunFunction(DeactivateProfiles, hWnd, hInst, lpszCmdLine, nCmdShow);
}

extern "C" __declspec(dllexport) HRESULT __cdecl IsProfileActivated() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> profileMgr;
    hr = profileMgr.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr.CoCreateInstance failed");

    TF_INPUTPROCESSORPROFILE profile;
    hr = profileMgr->GetProfile(
        TF_PROFILETYPE_INPUTPROCESSOR,
        VietType::Globals::TextServiceLangId,
        VietType::Globals::CLSID_TextService,
        VietType::Globals::GUID_Profile,
        NULL,
        &profile);
    HRESULT_CHECK_RETURN(hr, L"%s", L"profileMgr->GetProfile failed");

    return (profile.dwFlags & TF_IPP_FLAG_ENABLED) ? S_OK : S_FALSE;
}
