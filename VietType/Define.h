// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"
#include "resource.h"

#define TEXTSERVICE_MODEL L"Apartment"
#define TEXTSERVICE_LANGID MAKELANGID(LANG_VIETNAMESE, SUBLANG_VIETNAMESE_VIETNAM)
#define TEXTSERVICE_ICON_INDEX -IDIS_SAMPLEIME

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

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) ((sizeof(a) / sizeof(*(a))) / static_cast<size_t>(!(sizeof(a) % sizeof(*(a)))))
#endif
