// TfDumper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Common.h"

#define CHECK(hr, fmt, ...) if (FAILED(hr)) { wprintf(L"HRESULT error %lx: " fmt "\n", hr, __VA_ARGS__); return hr; }

std::wstring CppStringFromIID(const IID& iid) {
    CComHeapPtr<OLECHAR> ret;
    HRESULT hr = StringFromCLSID(iid, &ret);
    if (FAILED(hr)) {
        return std::wstring(L"FAILED");
    }
    return std::wstring(ret);
}

void PrintProfileInfo(const TF_INPUTPROCESSORPROFILE* p) {
    if (p->dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR) {
        wprintf(L"TF_PROFILETYPE_INPUTPROCESSOR ");
    } else if (p->dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT) {
        wprintf(L"TF_PROFILETYPE_KEYBOARDLAYOUT ");
    }
    wprintf(L"0x%04x:\n", static_cast<int>(p->langid));

    wprintf(L"clsid: %s, guidProfile: %s\n", CppStringFromIID(p->clsid).c_str(), CppStringFromIID(p->guidProfile).c_str());

    std::wstring catid = CppStringFromIID(p->catid);
    if (IsEqualGUID(p->catid, GUID_TFCAT_TIP_KEYBOARD)) {
        wprintf(L"catid: %s (GUID_TFCAT_TIP_KEYBOARD)\n", catid.c_str());
    } else if (IsEqualGUID(p->catid, GUID_TFCAT_TIP_SPEECH)) {
        wprintf(L"catid: %s (GUID_TFCAT_TIP_SPEECH)\n", catid.c_str());
    } else if (IsEqualGUID(p->catid, GUID_TFCAT_TIP_HANDWRITING)) {
        wprintf(L"catid: %s (GUID_TFCAT_TIP_HANDWRITING)\n", catid.c_str());
    } else if (IsEqualGUID(p->catid, GUID_TFCAT_CATEGORY_OF_TIP)) {
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
    wprintf(L"capabilities: %s\n", capabilities.c_str());

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
    wprintf(L"flags: %s\n", flags.c_str());

    wprintf(L"\n");
}

HRESULT EnumProfileMgr() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfileMgr> pm;
    hr = pm.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    CHECK(hr, L"%s", L"pm.CoCreate");

    CComPtr<IEnumTfInputProcessorProfiles> profiles;
    hr = pm->EnumProfiles(0, &profiles);
    CHECK(hr, L"%s", L"pm->EnumProfiles");

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

HRESULT EnumDefaultKeyboardLanguageProfiles() {
    HRESULT hr;

    CComPtr<ITfInputProcessorProfiles> profiles;
    hr = profiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    CHECK(hr, L"%s", L"profiles.CoCreate");

    LANGID curlangid;
    hr = profiles->GetCurrentLanguage(&curlangid);
    CHECK(hr, L"%s", L"profiles->GetCurrentLanguage");
    wprintf(L"Current language: 0x%04x\n", curlangid);

    wprintf(L"Default keyboard language profiles:\n");

    CComHeapPtr<LANGID> langids;
    ULONG numlangids;
    hr = profiles->GetLanguageList(&langids, &numlangids);
    CHECK(hr, L"%s", L"profiles->GetLanguageList");

    for (ULONG i = 0; i < numlangids; i++) {
        LANGID lid = (static_cast<LANGID*>(langids))[i];
        wprintf(L"0x%04x: ", lid);
        CLSID clsid;
        GUID guidProfile;
        hr = profiles->GetDefaultLanguageProfile(lid, GUID_TFCAT_TIP_KEYBOARD, &clsid, &guidProfile);
        CHECK(hr, L"%s", L"profiles->GetDefaultLanguageProfile");
        if (hr == S_FALSE) {
            wprintf(L"no default\n");
        } else {
            wprintf(L"clsid %s, guidProfile %s\n", CppStringFromIID(clsid).c_str(), CppStringFromIID(guidProfile).c_str());
        }
    }

    wprintf(L"\n");

    return S_OK;
}

int main() {
    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CHECK(hr, L"%s", L"CoInitializeEx");

    hr = EnumProfileMgr();
    CHECK(hr, L"%s", L"EnumProfileMgr");

    hr = EnumDefaultKeyboardLanguageProfiles();
    CHECK(hr, L"%s", L"EnumDefaultKeyboardLanguageProfiles");

    CoUninitialize();

    return S_OK;
}
