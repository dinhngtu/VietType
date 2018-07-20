// TfDumper.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Common.h"

#define CHECK(hr, fmt, ...) if (FAILED(hr)) { wprintf(L"HRESULT error %lx: " fmt "\n", hr, __VA_ARGS__); return hr; }

void PrintProfileInfo(TF_INPUTPROCESSORPROFILE const *p) {
    if (p->dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR) {
        wprintf(L"TF_PROFILETYPE_INPUTPROCESSOR ");
    } else if (p->dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT) {
        wprintf(L"TF_PROFILETYPE_KEYBOARDLAYOUT ");
    }
    wprintf(L"0x%x:\n", static_cast<int>(p->langid));

    CComHeapPtr<OLECHAR> clsid;
    StringFromCLSID(p->clsid, &clsid);
    CComHeapPtr<OLECHAR> guidProfile;
    StringFromCLSID(p->guidProfile, &guidProfile);
    wprintf(L"clsid %s guidProfile %s\n", static_cast<PWSTR>(clsid), static_cast<PWSTR>(guidProfile));

    CComHeapPtr<OLECHAR> catid;
    StringFromCLSID(p->catid, &catid);
    if (IsEqualGUID(p->catid, GUID_TFCAT_TIP_KEYBOARD)) {
        wprintf(L"catid %s (GUID_TFCAT_TIP_KEYBOARD)\n", static_cast<PWSTR>(catid));
    } else if (IsEqualGUID(p->catid, GUID_TFCAT_TIP_SPEECH)) {
        wprintf(L"catid %s (GUID_TFCAT_TIP_SPEECH)\n", static_cast<PWSTR>(catid));
    } else if (IsEqualGUID(p->catid, GUID_TFCAT_TIP_HANDWRITING)) {
        wprintf(L"catid %s (GUID_TFCAT_TIP_HANDWRITING)\n", static_cast<PWSTR>(catid));
    } else if (IsEqualGUID(p->catid, GUID_TFCAT_CATEGORY_OF_TIP)) {
        wprintf(L"catid %s (GUID_TFCAT_CATEGORY_OF_TIP)\n", static_cast<PWSTR>(catid));
    } else {
        wprintf(L"catid %s (unknown category)\n", static_cast<PWSTR>(catid));
    }

    wprintf(L"hkl %p\n", p->hkl);

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

    wprintf(L"hklSubstitute %p\n", p->hklSubstitute);

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

    SmartComPtr<ITfInputProcessorProfileMgr> pm;
    hr = pm.CoCreate(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER);
    CHECK(hr, L"%s", L"pm.CoCreate");

    SmartComPtr<IEnumTfInputProcessorProfiles> profiles;
    hr = pm->EnumProfiles(0, profiles.GetAddress());
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

int main() {
    HRESULT hr;

    hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    CHECK(hr, L"%s", L"CoInitializeEx");

    hr = EnumProfileMgr();
    CHECK(hr, L"%s", L"EnumProfileMgr");

    CoUninitialize();

    return S_OK;
}
