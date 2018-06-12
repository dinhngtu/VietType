// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#include "stdafx.h"

void DllAddRef();
void DllRelease();

namespace Global {

    //---------------------------------------------------------------------
    // extern
    //---------------------------------------------------------------------
    extern HINSTANCE dllInstanceHandle;

    extern LONG dllRefCount;

    extern CRITICAL_SECTION CS;

    extern const CLSID IMECLSID;
    extern const CLSID IMEGuidProfile;

    extern const GUID IME_PreservedKey_ToggleDisabled;

} // namespace Global

#ifdef _DEBUG
template <typename... Args>
void dbgprint(const wchar_t *func, int line, const wchar_t *fmt, Args... args) {
    // Format the string, maybe with vsprintf, log it, etc.
    wchar_t buf[2048];
    StringCchPrintf(buf, 2048, fmt, args...);
    //wchar_t buf2[2048];
    //StringCchPrintf(buf2, 2048, L"%s:%s: %s", func, line, buf);
    OutputDebugString(buf);
}
#define DBGPRINT(fmt, ...) dbgprint(__FUNCTIONW__, __LINE__, fmt, __VA_ARGS__)
#else
#define DBGPRINT(fmt, ...)
#endif
