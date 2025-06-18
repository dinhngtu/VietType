// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Common.h"
#include "Compartment.h"

#define CHECK(hr, fmt, ...)                                                                                            \
    if (FAILED(hr)) {                                                                                                  \
        wprintf(L"HRESULT error %lx: " fmt "\n", hr, __VA_ARGS__);                                                     \
        return hr;                                                                                                     \
    }

static std::wstring CppStringFromIID(const IID& iid) {
    CComHeapPtr<OLECHAR> ret;
    HRESULT hr = StringFromCLSID(iid, &ret);
    if (FAILED(hr)) {
        return std::wstring(L"FAILED");
    }
    return std::wstring(ret);
}

static void PrintProfileInfo(const TF_INPUTPROCESSORPROFILE* p) {
    if (p->dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR) {
        wprintf(L"TF_PROFILETYPE_INPUTPROCESSOR ");
    } else if (p->dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT) {
        wprintf(L"TF_PROFILETYPE_KEYBOARDLAYOUT ");
    }
    wprintf(L"0x%04x:\n", static_cast<int>(p->langid));

    auto clsidString = CppStringFromIID(p->clsid);
    wprintf(L"clsid: %s", clsidString.c_str());
    {
        CRegKey clsKey;
        auto keyPath = std::wstring(L"CLSID\\") + clsidString;
        LSTATUS status;
        status = clsKey.Open(HKEY_CLASSES_ROOT, keyPath.c_str(), KEY_READ);
        if (status == ERROR_SUCCESS) {
            {
                CRegKey serverKey;
                status = serverKey.Open(clsKey, L"InprocServer32", KEY_READ);
                if (status == ERROR_SUCCESS) {
                    ULONG dllPathSize;
                    serverKey.QueryStringValue(NULL, NULL, &dllPathSize);
                    std::vector<wchar_t> dllPath(dllPathSize);
                    status = serverKey.QueryStringValue(NULL, dllPath.data(), &dllPathSize);
                    if (status == ERROR_SUCCESS) {
                        wprintf(L" %.*s", dllPathSize, dllPath.data());
                    }
                }
            }
            {
                ULONG descSize;
                status = clsKey.QueryStringValue(NULL, NULL, &descSize);
                std::vector<wchar_t> desc(descSize);
                status = clsKey.QueryStringValue(NULL, desc.data(), &descSize);
                if (status == ERROR_SUCCESS) {
                    wprintf(L" \"%.*s\"", descSize, desc.data());
                }
            }
        }
        wprintf(L"\n");
    }
    wprintf(L"guidProfile: %s\n", CppStringFromIID(p->guidProfile).c_str());

    std::wstring catid = CppStringFromIID(p->catid);
    if (p->catid == GUID_TFCAT_TIP_KEYBOARD) {
        wprintf(L"catid: %s (GUID_TFCAT_TIP_KEYBOARD)\n", catid.c_str());
    } else if (p->catid == GUID_TFCAT_TIP_SPEECH) {
        wprintf(L"catid: %s (GUID_TFCAT_TIP_SPEECH)\n", catid.c_str());
    } else if (p->catid == GUID_TFCAT_TIP_HANDWRITING) {
        wprintf(L"catid: %s (GUID_TFCAT_TIP_HANDWRITING)\n", catid.c_str());
    } else if (p->catid == GUID_TFCAT_CATEGORY_OF_TIP) {
        wprintf(L"catid: %s (GUID_TFCAT_CATEGORY_OF_TIP)\n", catid.c_str());
    } else {
        wprintf(L"catid: %s (unknown category)\n", catid.c_str());
    }

    wprintf(L"hkl: %p\n", p->hkl);

    std::wstring capabilities;
    if (p->dwCaps & TF_IPP_CAPS_DISABLEONTRANSITORY) {
        capabilities += L"DISABLEONTRANSITORY ";
    }
    if (p->dwCaps & TF_IPP_CAPS_SECUREMODESUPPORT) {
        capabilities += L"SECUREMODESUPPORT ";
    }
    if (p->dwCaps & TF_IPP_CAPS_UIELEMENTENABLED) {
        capabilities += L"UIELEMENTENABLED ";
    }
    if (p->dwCaps & TF_IPP_CAPS_COMLESSSUPPORT) {
        capabilities += L"COMLESSSUPPORT ";
    }
    if (p->dwCaps & TF_IPP_CAPS_WOW16SUPPORT) {
        capabilities += L"WOW16SUPPORT ";
    }
    if (p->dwCaps & TF_IPP_CAPS_IMMERSIVESUPPORT) {
        capabilities += L"IMMERSIVESUPPORT ";
    }
    if (p->dwCaps & TF_IPP_CAPS_SYSTRAYSUPPORT) {
        capabilities += L"SYSTRAYSUPPORT ";
    }
    wprintf(L"capabilities: %lx %s\n", p->dwCaps, capabilities.c_str());

    wprintf(L"hklSubstitute: %p\n", p->hklSubstitute);

    std::wstring flags;
    if (p->dwFlags & TF_IPP_FLAG_ACTIVE) {
        flags += L"ACTIVE ";
    }
    if (p->dwFlags & TF_IPP_FLAG_ENABLED) {
        flags += L"ENABLED ";
    }
    if (p->dwFlags & TF_IPP_FLAG_SUBSTITUTEDBYINPUTPROCESSOR) {
        flags += L"SUBSTITUTEDBYINPUTPROCESSOR ";
    }
    wprintf(L"flags: %lx %s\n", p->dwFlags, flags.c_str());

    wprintf(L"\n");
}

static HRESULT EnumProfileMgr() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> pm;
    hr = pm.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    CHECK(hr, L"pm.CoCreate");

    CComPtr<IEnumTfInputProcessorProfiles> profiles;
    hr = pm->EnumProfiles(0, &profiles);
    CHECK(hr, L"pm->EnumProfiles");

    TF_INPUTPROCESSORPROFILE p;
    while (1) {
        ULONG found;
        hr = profiles->Next(1, &p, &found);
        if (FAILED(hr) || !found) {
            break;
        }
        PrintProfileInfo(&p);
    }

    hr = pm->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &p);
    wprintf(L"Active profile: ");
    PrintProfileInfo(&p);

    return S_OK;
}

static HRESULT EnumDefaultKeyboardLanguageProfiles() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfiles> profiles;
    hr = profiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    CHECK(hr, L"profiles.CoCreate");

    LANGID curlangid;
    hr = profiles->GetCurrentLanguage(&curlangid);
    CHECK(hr, L"profiles->GetCurrentLanguage");
    wprintf(L"Current language: 0x%04x\n", curlangid);

    wprintf(L"Default keyboard language profiles:\n");

    CComHeapPtr<LANGID> langids;
    ULONG numlangids;
    hr = profiles->GetLanguageList(&langids, &numlangids);
    CHECK(hr, L"profiles->GetLanguageList");

    for (ULONG i = 0; i < numlangids; i++) {
        LANGID lid = (static_cast<LANGID*>(langids))[i];
        wprintf(L"0x%04x: ", lid);
        CLSID clsid;
        GUID guidProfile;
        hr = profiles->GetDefaultLanguageProfile(lid, GUID_TFCAT_TIP_KEYBOARD, &clsid, &guidProfile);
        CHECK(hr, L"profiles->GetDefaultLanguageProfile");
        if (hr == S_FALSE) {
            wprintf(L"no default\n");
        } else {
            wprintf(
                L"clsid %s, guidProfile %s\n", CppStringFromIID(clsid).c_str(), CppStringFromIID(guidProfile).c_str());
        }
    }

    wprintf(L"\n");

    return S_OK;
}

enum TfDumperModes {
    DefaultMode,
    ProfileMode,
    ImmStatusMode,
};

static HRESULT doit_immstatus() {
    HRESULT hr = S_OK;
    HWND hwnd = NULL;
    LRESULT status;

    hwnd = ImmGetDefaultIMEWnd(GetForegroundWindow());
    if (hwnd == NULL) {
        hr = E_FAIL;
        wprintf(L"ImmGetDefaultIMEWnd\n");
        goto fail;
    }

    status = SendMessage(hwnd, WM_IME_CONTROL, 5 /* IMC_GETOPENSTATUS */, 0);
    wprintf(L"openstatus %llx\n", status);

    status = SendMessage(hwnd, WM_IME_CONTROL, 1 /* IMC_GETCONVERSIONMODE */, 0);
    wprintf(L"conversionmode %llx\n", status);
    if (status & IME_CMODE_NATIVE)
        wprintf(L"IME_CMODE_NATIVE\n");
    if (status & IME_CMODE_KATAKANA)
        wprintf(L"IME_CMODE_KATAKANA\n");
    if (status & IME_CMODE_FULLSHAPE)
        wprintf(L"IME_CMODE_FULLSHAPE\n");
    if (status & IME_CMODE_ROMAN)
        wprintf(L"IME_CMODE_ROMAN\n");
    if (status & IME_CMODE_CHARCODE)
        wprintf(L"IME_CMODE_CHARCODE\n");
    if (status & IME_CMODE_HANJACONVERT)
        wprintf(L"IME_CMODE_HANJACONVERT\n");
    if (status & IME_CMODE_NATIVESYMBOL)
        wprintf(L"IME_CMODE_NATIVESYMBOL\n");
    if (status & IME_CMODE_NOCONVERSION)
        wprintf(L"IME_CMODE_NOCONVERSION\n");
    if (status & IME_CMODE_EUDC)
        wprintf(L"IME_CMODE_EUDC\n");
    if (status & IME_CMODE_SYMBOL)
        wprintf(L"IME_CMODE_SYMBOL\n");
    if (status & IME_CMODE_FIXED)
        wprintf(L"IME_CMODE_FIXED\n");

    return S_OK;

fail:
    return hr;
}

static HRESULT doit(int mode) {
    HRESULT hr = S_OK;

    switch (mode) {
    case ProfileMode:
        hr = EnumProfileMgr();
        CHECK(hr, L"EnumProfileMgr");

        hr = EnumDefaultKeyboardLanguageProfiles();
        CHECK(hr, L"EnumDefaultKeyboardLanguageProfiles");

        break;

    case ImmStatusMode:
        hr = doit_immstatus();
        CHECK(hr, L"doit_immstatus");
        break;

    default:
        hr = E_FAIL;
        CHECK(hr, L"Specify a mode\n");
        break;
    }

    return hr;
}

int wmain(int argc, wchar_t** argv) {
    HRESULT hr;
    int mode = DefaultMode;

    for (int i = 1; i < argc; i++) {
        if (!_wcsicmp(argv[i], L"profile"))
            mode = ProfileMode;
        else if (!_wcsicmp(argv[i], L"immstatus"))
            mode = ImmStatusMode;
    }

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CHECK(hr, L"CoInitializeEx");

    hr = doit(mode);
    CHECK(hr, L"doit %d", mode);

    CoUninitialize();

    return S_OK;
}
