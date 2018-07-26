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

// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

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


// support function, do not use directly
template <typename... Args>
void _dprintw(_In_ LPCWSTR func, _In_ int line, _In_ LPCWSTR fmt, Args... args) {
    std::array<WCHAR, 2048> buf;
    StringCchPrintfW(&buf[0], buf.size(), fmt, func, line, args...);
    OutputDebugStringW(&buf[0]);
}

// support function, do not use directly
template <typename... Args>
void _dprinta(_In_ LPCSTR func, _In_ int line, _In_ LPCSTR fmt, Args... args) {
    std::array<CHAR, 1024> buf;
    StringCchPrintfA(&buf[0], buf.size(), fmt, func, line, args...);
    OutputDebugStringA(&buf[0]);
}

// print formatted string to debugger output
#define DPRINTW(fmt, ...) _dprintw(__FUNCTIONW__, __LINE__, L"%s:%d: " fmt L"\n", __VA_ARGS__)
// print formatted string to debugger output
#define DPRINTA(fmt, ...) _dprinta(__FUNCTION__, __LINE__, "%s:%d: " fmt "\n", __VA_ARGS__)
// check if HRESULT is successful; if not, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN_W(hr, fmt, ...) if (FAILED(hr)) { DPRINTW(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); return hr; }
// check if HRESULT is successful; if not, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN_A(hr, fmt, ...) if (FAILED(hr)) { DPRINTA("HRESULT error %lx: " fmt, hr, __VA_ARGS__); return hr; }

#ifdef _DEBUG
// print formatted string to debugger output in debug builds
#define DBG_DPRINTW DPRINTW
// print formatted string to debugger output in debug builds
#define DBG_DPRINTA DPRINTA
// check if HRESULT is successful; if not, print error information to debugger output
#define DBG_HRESULT_CHECK_W(hr, fmt, ...) if (FAILED(hr)) { DBG_DPRINTW(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); }
// check if HRESULT is successful; if not, print error information to debugger output
#define DBG_HRESULT_CHECK_A(hr, fmt, ...) if (FAILED(hr)) { DBG_DPRINTA("HRESULT error %lx: " fmt, hr, __VA_ARGS__); }
#else
#define DBG_DPRINTW(fmt, ...)
#define DBG_DPRINTA(fmt, ...)
#define DBG_HRESULT_CHECK_W(hr, fmt, ...)
#define DBG_HRESULT_CHECK_A(hr, fmt, ...)
#endif

#ifdef UNICODE
#define HRESULT_CHECK_RETURN HRESULT_CHECK_RETURN_W
#define DBG_DPRINT DBG_DPRINTW
#define DBG_HRESULT_CHECK DBG_HRESULT_CHECK_W
#else
#define HRESULT_CHECK_RETURN HRESULT_CHECK_RETURN_A
#define DBG_DPRINT DBG_DPRINTA
#define DBG_HRESULT_CHECK DBG_HRESULT_CHECK_A
#endif


// support function, do not use directly
template <typename... Args>
void _winerrorprint(_In_ LPCWSTR func, _In_ int line, _In_ DWORD err, _In_ LPCWSTR fmt, Args... args) {
    std::array<WCHAR, 256> errmessage;
    auto chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, err, 0, &errmessage[0], static_cast<DWORD>(errmessage.size()), NULL);
    assert(chars >= 0 && chars < 256);
    errmessage[chars] = 0;

    std::array<WCHAR, 2048> buf;
    StringCchPrintf(&buf[0], buf.size(), fmt, func, line, err, errmessage, args...);
    OutputDebugString(&buf[0]);
}

// format win32 error and print formatted string to debugger output
#define WINERROR_PRINT(err, fmt, ...) _winerrorprint(__FUNCTIONW__, __LINE__, err, L"%s:%d: WINERROR %lx (%s): " fmt L"\n", __VA_ARGS__)
// format win32 error, print formatted string to debugger output, and return from calling function with error converted to HRESULT
#define WINERROR_RETURN_HRESULT(fmt, ...) do { auto err = GetLastError(); WINERROR_PRINT(err, fmt, __VA_ARGS__); return HRESULT_FROM_WIN32(err); } while (0)
