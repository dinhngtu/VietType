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


/// <summary>support function, do not use directly</summary>
template <typename... Args>
void _dprint(_In_ LPCWSTR func, _In_ int line, _In_ LPCWSTR fmt, Args... args) {
    std::array<WCHAR, 2048> buf;
    StringCchPrintf(&buf[0], buf.size(), fmt, func, line, args...);
    OutputDebugString(&buf[0]);
}

// print formatted string to debugger output
#define DPRINT(fmt, ...) _dprint(__FUNCTIONW__, __LINE__, L"%s:%d: " fmt L"\n", __VA_ARGS__)
// check if HRESULT is successful; if not, print error information to debugger output
#define HRESULT_CHECK(hr, fmt, ...) if (FAILED(hr)) { DPRINT(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); }
// check if HRESULT is successful; if not, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN(hr, fmt, ...) if (FAILED(hr)) { DPRINT(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); return hr; }

#ifdef _DEBUG
// if in debug builds, print formatted string to debugger output
#define DBG_DPRINT DPRINT
// if in debug builds, check if HRESULT is successful; if not, print error information to debugger output
#define DBG_HRESULT_CHECK(hr, fmt, ...) if (FAILED(hr)) { DBG_DPRINT(L"HRESULT error %lx: " fmt, hr, __VA_ARGS__); }
#else
#define DBG_DPRINT(fmt, ...)
#define DBG_HRESULT_CHECK(hr, fmt, ...)
#endif


/// <summary>support function, do not use directly</summary>
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
