// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "stdafx.h"


// Put this in the declarations for a class to be uncopyable.
#define DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete

// Put this in the declarations for a class to be unassignable.
#define DISALLOW_ASSIGN(TypeName) TypeName& operator=(const TypeName&) = delete

// Put this in the declarations for a class to be uncopyable and unassignable.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) DISALLOW_COPY(TypeName); DISALLOW_ASSIGN(TypeName)

// A macro to disallow all the implicit constructors, namely the
// default constructor, copy constructor and operator= functions.
// This is especially useful for classes containing only static methods.
#define DISALLOW_IMPLICIT_CONSTRUCTORS(TypeName) TypeName() = delete; DISALLOW_COPY_AND_ASSIGN(TypeName)


/// <summary>support function, do not use directly</summary>
template <typename... Args>
void _dprint(const wchar_t *func, int line, const wchar_t *fmt, Args... args) {
    wchar_t buf[2048];
    StringCchPrintf(buf, 2048, fmt, func, line, args...);
    OutputDebugString(buf);
}

// print formatted string to debugger output
#define DPRINT(fmt, ...) _dprint(__FUNCTIONW__, __LINE__, L"%s:%d: " fmt, __VA_ARGS__)
// check if HRESULT is successful; if not, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN(hr, fmt, ...) if (FAILED(hr)) { DPRINT(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); return hr; }

#ifdef _DEBUG
// print formatted string to debugger output in debug builds
#define DBG_DPRINT DPRINT
// check if HRESULT is successful; if not, print error information to debugger output
#define DBG_HRESULT_CHECK(hr, fmt, ...) if (FAILED(hr)) { DBG_DPRINT(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); }
#else
#define DBG_DPRINT(fmt, ...)
#define DBG_HRESULT_CHECK(hr, fmt, ...)
#endif


/// <summary>support function, do not use directly</summary>
template <typename... Args>
void _winerrorprint(const wchar_t *func, int line, DWORD err, const wchar_t *fmt, Args... args) {
    wchar_t errmessage[256];
    auto chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, errmessage, 256, NULL);

    wchar_t buf[2048];
    StringCchPrintf(buf, 2048, fmt, func, line, err, errmessage, args...);
    OutputDebugString(buf);
}

// format win32 error and print formatted string to debugger output
#define WINERROR_PRINT(err, fmt, ...) _winerrorprint(__FUNCTIONW__, __LINE__, err, L"%s:%d: WINERROR %lx (%s): " fmt, __VA_ARGS__)
// format win32 error, print formatted string to debugger output, and return from calling function with error converted to HRESULT
#define WINERROR_RETURN_HRESULT(fmt, ...) do { auto err = GetLastError(); WINERROR_PRINT(err, fmt, __VA_ARGS__); return HRESULT_FROM_WIN32(err); } while (0)
