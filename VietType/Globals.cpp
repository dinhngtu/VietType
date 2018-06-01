// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "stdafx.h"

namespace Global {
HINSTANCE dllInstanceHandle;

LONG dllRefCount = -1;

CRITICAL_SECTION CS;
HFONT defaultlFontHandle; // Global font object we use everywhere

//---------------------------------------------------------------------
// SampleIME CLSID
//---------------------------------------------------------------------
// {D2291A80-84D8-4641-9AB2-BDD1472C846B}
extern const CLSID SampleIMECLSID = {
    0xd2291a80, 0x84d8, 0x4641, {0x9a, 0xb2, 0xbd, 0xd1, 0x47, 0x2c, 0x84, 0x6b}};

//---------------------------------------------------------------------
// Profile GUID
//---------------------------------------------------------------------
// {83955C0E-2C09-47a5-BCF3-F2B98E11EE8B}
extern const GUID SampleIMEGuidProfile = {
    0x83955c0e, 0x2c09, 0x47a5, {0xbc, 0xf3, 0xf2, 0xb9, 0x8e, 0x11, 0xee, 0x8b}};

} // namespace Global
