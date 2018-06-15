// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"
#include "resource.h"

#define TEXTSERVICE_DISPLAYNAME L"VietType"

#define TEXTSERVICE_MODEL L"Apartment"

//#define TEXTSERVICE_LANGID MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM)
// HACK: A Vietnamese LANGID forces us to use the horrible KBDVNTC.DLL keyboard layout, which is not compatible with Telex.
// Using English (US) lets us define a text service based on the QWERTY layout without having to remap keys.
#define TEXTSERVICE_LANGID MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US)

#define TEXTSERVICE_ICON_INDEX -IDIS_IMELOGO

#define IME_MODE_ON_ICON_INDEX IDI_IME_MODE_ON
#define IME_MODE_OFF_ICON_INDEX IDI_IME_MODE_OFF
#define IME_DOUBLE_ON_INDEX IDI_DOUBLE_SINGLE_BYTE_ON
#define IME_DOUBLE_OFF_INDEX IDI_DOUBLE_SINGLE_BYTE_OFF
#define IME_PUNCTUATION_ON_INDEX IDI_PUNCTUATION_ON
#define IME_PUNCTUATION_OFF_INDEX IDI_PUNCTUATION_OFF

//---------------------------------------------------------------------
// string length of CLSID
//---------------------------------------------------------------------
#define CLSID_STRLEN (38) // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")
