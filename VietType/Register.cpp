// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Define.h"
#include "Globals.h"
#include "stdafx.h"

static const WCHAR RegInfo_Prefix_CLSID[] = L"CLSID\\";
static const WCHAR RegInfo_Key_InProSvr32[] = L"InProcServer32";
static const WCHAR RegInfo_Key_ThreadModel[] = L"ThreadingModel";

static const WCHAR TEXTSERVICE_DESC[] = L"Sample IME";

static const GUID SupportCategories[] = {
    GUID_TFCAT_TIP_KEYBOARD,
    GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER,
    GUID_TFCAT_TIPCAP_UIELEMENTENABLED,
    GUID_TFCAT_TIPCAP_SECUREMODE,
    GUID_TFCAT_TIPCAP_COMLESS,
    GUID_TFCAT_TIPCAP_INPUTMODECOMPARTMENT,
    GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT,
    GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT,
};
static const int SupportCategoriesCount = 8;

const BYTE GuidSymbols[] = { 3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15 };

const WCHAR HexDigits[] = L"0123456789ABCDEF";

BOOL CLSIDToString(REFGUID refGUID, _Out_writes_(39) WCHAR *pCLSIDString) {
    WCHAR *pTemp = pCLSIDString;
    const BYTE *pBytes = (const BYTE *)&refGUID;

    DWORD j = 0;
    pTemp[j++] = L'{';
    for (int i = 0; i < sizeof(GuidSymbols) && j < (CLSID_STRLEN - 2); i++) {
        if (GuidSymbols[i] == '-') {
            pTemp[j++] = L'-';
        } else {
            pTemp[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0xF0) >> 4];
            pTemp[j++] = HexDigits[(pBytes[GuidSymbols[i]] & 0x0F)];
        }
    }

    pTemp[j++] = L'}';
    pTemp[j] = L'\0';

    return TRUE;
}
//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

BOOL RegisterProfiles() {
    DBGPRINT(L"RegisterProfile langid %u", TEXTSERVICE_LANGID);
    HRESULT hr = S_FALSE;

    ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;
    hr = CoCreateInstance(
        CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfileMgr,
        (void **)&pITfInputProcessorProfileMgr);
    if (FAILED(hr)) {
        return FALSE;
    }

    WCHAR achIconFile[MAX_PATH] = { '\0' };
    DWORD cchA = 0;
    cchA = GetModuleFileName(Global::dllInstanceHandle, achIconFile, MAX_PATH);
    cchA = cchA >= MAX_PATH ? (MAX_PATH - 1) : cchA;
    achIconFile[cchA] = '\0';

    /*
    HKL hkl = LoadKeyboardLayout(L"a000042a", 0);
    DBGPRINT(L"RegisterProfiles hkl %zu", hkl);
    */

    size_t lenOfDesc = 0;
    hr = StringCchLength(TEXTSERVICE_DESC, STRSAFE_MAX_CCH, &lenOfDesc);
    if (hr != S_OK) {
        goto Exit;
    }
    hr = pITfInputProcessorProfileMgr->RegisterProfile(
        Global::IMECLSID,
        TEXTSERVICE_LANGID,
        Global::IMEGuidProfile,
        TEXTSERVICE_DESC,
        static_cast<ULONG>(lenOfDesc),
        achIconFile,
        cchA,
        (UINT)TEXTSERVICE_ICON_INDEX,
        NULL,
        0,
        TRUE,
        0);

    DBGPRINT(L"RegisterProfile hr %ld", hr);

    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    if (pITfInputProcessorProfileMgr) {
        pITfInputProcessorProfileMgr->Release();
    }

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterProfiles
//
//----------------------------------------------------------------------------

void UnregisterProfiles() {
    HRESULT hr = S_OK;

    ITfInputProcessorProfileMgr *pITfInputProcessorProfileMgr = nullptr;
    hr = CoCreateInstance(
        CLSID_TF_InputProcessorProfiles,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfInputProcessorProfileMgr,
        (void **)&pITfInputProcessorProfileMgr);
    if (FAILED(hr)) {
        goto Exit;
    }

    hr = pITfInputProcessorProfileMgr->UnregisterProfile(
        Global::IMECLSID, TEXTSERVICE_LANGID, Global::IMEGuidProfile, 0);
    if (FAILED(hr)) {
        goto Exit;
    }

Exit:
    if (pITfInputProcessorProfileMgr) {
        pITfInputProcessorProfileMgr->Release();
    }

    return;
}

//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL RegisterCategories() {
    ITfCategoryMgr *pCategoryMgr = nullptr;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        CLSID_TF_CategoryMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfCategoryMgr,
        (void **)&pCategoryMgr);
    if (FAILED(hr)) {
        return FALSE;
    }

    for (int i = 0; i < SupportCategoriesCount; i++) {
        hr = pCategoryMgr->RegisterCategory(
            Global::IMECLSID, SupportCategories[i], Global::IMECLSID);
    }

    pCategoryMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterCategories
//
//----------------------------------------------------------------------------

void UnregisterCategories() {
    ITfCategoryMgr *pCategoryMgr = S_OK;
    HRESULT hr = S_OK;

    hr = CoCreateInstance(
        CLSID_TF_CategoryMgr,
        NULL,
        CLSCTX_INPROC_SERVER,
        IID_ITfCategoryMgr,
        (void **)&pCategoryMgr);
    if (FAILED(hr)) {
        return;
    }

    for (int i = 0; i < SupportCategoriesCount; i++) {
        pCategoryMgr->UnregisterCategory(
            Global::IMECLSID, SupportCategories[i], Global::IMECLSID);
    }

    pCategoryMgr->Release();

    return;
}

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------

LONG RecurseDeleteKey(_In_ HKEY hParentKey, _In_ LPCTSTR lpszKey) {
    HKEY regKeyHandle = nullptr;
    LONG res = 0;
    FILETIME time;
    WCHAR stringBuffer[256] = { '\0' };
    DWORD size = ARRAYSIZE(stringBuffer);

    if (RegOpenKey(hParentKey, lpszKey, &regKeyHandle) != ERROR_SUCCESS) {
        return ERROR_SUCCESS;
    }

    res = ERROR_SUCCESS;
    while (RegEnumKeyEx(regKeyHandle, 0, stringBuffer, &size, NULL, NULL, NULL, &time) ==
        ERROR_SUCCESS) {
        stringBuffer[ARRAYSIZE(stringBuffer) - 1] = '\0';
        res = RecurseDeleteKey(regKeyHandle, stringBuffer);
        if (res != ERROR_SUCCESS) {
            break;
        }
        size = ARRAYSIZE(stringBuffer);
    }
    RegCloseKey(regKeyHandle);

    return res == ERROR_SUCCESS ? RegDeleteKey(hParentKey, lpszKey) : res;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL RegisterServer() {
    DWORD copiedStringLen = 0;
    HKEY regKeyHandle = nullptr;
    HKEY regSubkeyHandle = nullptr;
    BOOL ret = FALSE;
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = { '\0' };
    WCHAR achFileName[MAX_PATH] = { '\0' };

    if (!CLSIDToString(Global::IMECLSID, achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1)) {
        return FALSE;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    if (RegCreateKeyEx(
        HKEY_CLASSES_ROOT,
        achIMEKey,
        0,
        NULL,
        REG_OPTION_NON_VOLATILE,
        KEY_WRITE,
        NULL,
        &regKeyHandle,
        &copiedStringLen) == ERROR_SUCCESS) {
        if (RegSetValueEx(
            regKeyHandle,
            NULL,
            0,
            REG_SZ,
            (const BYTE *)TEXTSERVICE_DESC,
            (_countof(TEXTSERVICE_DESC)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
            goto Exit;
        }

        if (RegCreateKeyEx(
            regKeyHandle,
            RegInfo_Key_InProSvr32,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_WRITE,
            NULL,
            &regSubkeyHandle,
            &copiedStringLen) == ERROR_SUCCESS) {
            copiedStringLen =
                GetModuleFileNameW(Global::dllInstanceHandle, achFileName, ARRAYSIZE(achFileName));
            copiedStringLen = (copiedStringLen >= (MAX_PATH - 1)) ? MAX_PATH : (++copiedStringLen);
            if (RegSetValueEx(
                regSubkeyHandle,
                NULL,
                0,
                REG_SZ,
                (const BYTE *)achFileName,
                (copiedStringLen) * sizeof(WCHAR)) != ERROR_SUCCESS) {
                goto Exit;
            }
            if (RegSetValueEx(
                regSubkeyHandle,
                RegInfo_Key_ThreadModel,
                0,
                REG_SZ,
                (const BYTE *)TEXTSERVICE_MODEL,
                (_countof(TEXTSERVICE_MODEL)) * sizeof(WCHAR)) != ERROR_SUCCESS) {
                goto Exit;
            }

            ret = TRUE;
        }
    }

Exit:
    if (regSubkeyHandle) {
        RegCloseKey(regSubkeyHandle);
        regSubkeyHandle = nullptr;
    }
    if (regKeyHandle) {
        RegCloseKey(regKeyHandle);
        regKeyHandle = nullptr;
    }

    return ret;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void UnregisterServer() {
    WCHAR achIMEKey[ARRAYSIZE(RegInfo_Prefix_CLSID) + CLSID_STRLEN] = { '\0' };

    if (!CLSIDToString(Global::IMECLSID, achIMEKey + ARRAYSIZE(RegInfo_Prefix_CLSID) - 1)) {
        return;
    }

    memcpy(achIMEKey, RegInfo_Prefix_CLSID, sizeof(RegInfo_Prefix_CLSID) - sizeof(WCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}
