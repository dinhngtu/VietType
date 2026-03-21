// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "stdafx.h"

namespace VietType::Globals {

extern HINSTANCE DllInstance;

// {c0dd01a1-0deb-454b-8b42-d22ced1b4b23}
DEFINE_GUID(
    CLSID_TextService, //
    0xc0dd01a1,
    0x0deb,
    0x454b,
    0x8b,
    0x42,
    0xd2,
    0x2c,
    0xed,
    0x1b,
    0x4b,
    0x23);

// {8D93D10A-203B-4C5F-A122-8898EF9C56F5}
DEFINE_GUID(
    GUID_Profile, //
    0x8d93d10a,
    0x203b,
    0x4c5f,
    0xa1,
    0x22,
    0x88,
    0x98,
    0xef,
    0x9c,
    0x56,
    0xf5);

DEFINE_GUID(
    GUID_VIETTYPE_LBI_INPUTMODE, //
    0x2C77A81E,
    0x41CC,
    0x4178,
    0xA3,
    0xA7,
    0x5F,
    0x8A,
    0x98,
    0x75,
    0x68,
    0xE6);

DEFINE_GUID(
    GUID_VIETTYPE_PROP_INPUTSCOPE, //
    0x1713dd5a,
    0x68e7,
    0x4a5b,
    0x9a,
    0xf6,
    0x59,
    0x2a,
    0x59,
    0x5c,
    0x77,
    0x8d);

// {8CC27CF8-93D2-416C-B1A3-66827F54244A}
DEFINE_GUID(
    GUID_KeyEventSink_PreservedKey_Toggle, //
    0x8cc27cf8,
    0x93d2,
    0x416c,
    0xb1,
    0xa3,
    0x66,
    0x82,
    0x7f,
    0x54,
    0x24,
    0x4a);

// {C36CF93E-2FE6-4132-92E5-7E7FB242D56D}
DEFINE_GUID(
    GUID_Compartment_Backconvert, //
    0xc36cf93e,
    0x2fe6,
    0x4132,
    0x92,
    0xe5,
    0x7e,
    0x7f,
    0xb2,
    0x42,
    0xd5,
    0x6d);

// {B31B741B-63CE-413A-9B5A-D2B69C695A78}
DEFINE_GUID(
    GUID_Compartment_EnabledToggle, //
    0xb31b741b,
    0x63ce,
    0x413a,
    0x9b,
    0x5a,
    0xd2,
    0xb6,
    0x9c,
    0x69,
    0x5a,
    0x78);

// {B2FBD2E7-922F-4996-BE77-21085B91A8F0}
DEFINE_GUID(
    GUID_Compartment_SystemNotify, //
    0xb2fbd2e7,
    0x922f,
    0x4996,
    0xbe,
    0x77,
    0x21,
    0x8,
    0x5b,
    0x91,
    0xa8,
    0xf0);

// {A94C5FD2-C471-4031-9546-709C17300CB9}
DEFINE_GUID(
    GUID_Compartment_TsfEmulatedDocumentMgr,
    0xa94c5fd2,
    0xc471,
    0x4031,
    0x95,
    0x46,
    0x70,
    0x9c,
    0x17,
    0x30,
    0x0c,
    0xb9);

extern const DECLSPEC_SELECTANY wchar_t TextServiceDescription[] = L"VietType";
extern const DECLSPEC_SELECTANY wchar_t ConfigKeyName[] = L"Software\\VietType";

constexpr LANGID TextServiceLangId = MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM);

extern const DECLSPEC_SELECTANY wchar_t SettingsProgSubpath[] = L"\\VietType\\VietTypeConfig2.exe";

} // namespace VietType::Globals
