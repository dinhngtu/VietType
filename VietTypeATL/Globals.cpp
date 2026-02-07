// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Globals.h"

namespace VietType {
namespace Globals {

HINSTANCE DllInstance = nullptr;

// {c0dd01a1-0deb-454b-8b42-d22ced1b4b23}
const CLSID CLSID_TextService = {0xc0dd01a1, 0x0deb, 0x454b, {0x8b, 0x42, 0xd2, 0x2c, 0xed, 0x1b, 0x4b, 0x23}};

// {8D93D10A-203B-4C5F-A122-8898EF9C56F5}
const GUID GUID_Profile = {0x8d93d10a, 0x203b, 0x4c5f, {0xa1, 0x22, 0x88, 0x98, 0xef, 0x9c, 0x56, 0xf5}};

// pass this as guidItem of language bar item to make it show on the input indicator (the one next to the clock) instead
// of the language bar
const GUID GUID_LBI_INPUTMODE = {0x2C77A81E, 0x41CC, 0x4178, {0xA3, 0xA7, 0x5F, 0x8A, 0x98, 0x75, 0x68, 0xE6}};

const GUID GUID_PROP_INPUTSCOPE = {0x1713dd5a, 0x68e7, 0x4a5b, {0x9a, 0xf6, 0x59, 0x2a, 0x59, 0x5c, 0x77, 0x8d}};

// {C36CF93E-2FE6-4132-92E5-7E7FB242D56D}
const GUID GUID_Compartment_Backconvert = {
    0xc36cf93e, 0x2fe6, 0x4132, {0x92, 0xe5, 0x7e, 0x7f, 0xb2, 0x42, 0xd5, 0x6d}};

// {B2FBD2E7-922F-4996-BE77-21085B91A8F0}
const GUID GUID_Compartment_SystemNotify = {0xb2fbd2e7, 0x922f, 0x4996, {0xbe, 0x77, 0x21, 0x8, 0x5b, 0x91, 0xa8, 0xf0}};

const wchar_t* TextServiceDescription = L"VietType";
const wchar_t* ConfigKeyName = L"Software\\VietType";

const LANGID TextServiceLangId = MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM);

const wchar_t* SettingsProgSubpath = L"\\VietType\\VietTypeConfig2.exe";

} // namespace Globals
} // namespace VietType
