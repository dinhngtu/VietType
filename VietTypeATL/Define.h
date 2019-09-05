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

#include <Windows.h>
#include <strsafe.h>


/// <summary>support function, do not use directly</summary>
template <typename... Args>
void _dprint(_In_ LPCWSTR func, _In_ int line, _In_ LPCWSTR fmt, Args... args) {
    std::array<WCHAR, 512> buf;
    StringCchPrintf(&buf[0], buf.size(), fmt, func, line, args...);
    OutputDebugString(&buf[0]);
}


/// <summary>support function, do not use directly</summary>
template <typename... Args>
void _errorprint(_In_ LPCWSTR func, _In_ int line, _In_ DWORD err, _In_ LPCWSTR fmt, Args... args) {
    std::array<WCHAR, 256> errmessage;
    auto chars = FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, 0, &errmessage[0], static_cast<DWORD>(errmessage.size()), NULL);
    assert(chars >= 0 && chars < 256);
    errmessage[chars] = 0;
    for (auto c = static_cast<long>(chars) - 1; c >= 0; c--) {
        if (errmessage[c] == L'\r' || errmessage[c] == L'\n') {
            errmessage[c] = 0;
        } else {
            break;
        }
    }

    std::array<WCHAR, 512> buf;
    StringCchPrintf(&buf[0], buf.size(), fmt, func, line, err, errmessage, args...);
    OutputDebugString(&buf[0]);
}


// print formatted string to debugger output
#define DPRINT(fmt, ...) _dprint(__FUNCTIONW__, __LINE__, L"%s:%d: " fmt L"\n", __VA_ARGS__)

#define HRESULT_PRINT(hr, fmt, ...) _errorprint(__FUNCTIONW__, __LINE__, hr, L"%s:%d: HRESULT error %lx (%s): " fmt, __VA_ARGS__)
// check if HRESULT is successful; if not, print error information to debugger output
#define HRESULT_CHECK(hr, fmt, ...) if (FAILED(hr)) { HRESULT_PRINT(hr, fmt, __VA_ARGS__); }
// check if HRESULT is successful; if not, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN(hr, fmt, ...) if (FAILED(hr)) { HRESULT_PRINT(hr, fmt, __VA_ARGS__); return hr; }
// check if HRESULT is successful; if not, set *ptr to nullptr, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN_OUTPTR(hr, ptr, fmt, ...) if (FAILED(hr)) { *ptr = nullptr; HRESULT_PRINT(hr, fmt, __VA_ARGS__); return hr; }

#ifdef _DEBUG
// if in debug builds, print formatted string to debugger output
#define DBG_DPRINT DPRINT
// if in debug builds, check if HRESULT is successful; if not, print error information to debugger output
#define DBG_HRESULT_CHECK HRESULT_CHECK
#else
#define DBG_DPRINT(fmt, ...)
#define DBG_HRESULT_CHECK(hr, fmt, ...)
#endif

// format win32 error and print formatted string to debugger output
#define WINERROR_PRINT(err, fmt, ...) _errorprint(__FUNCTIONW__, __LINE__, err, L"%s:%d: WINERROR %lx (%s): " fmt L"\n", __VA_ARGS__)
// format win32 error, print formatted string to debugger output, and return from calling function with error converted to HRESULT
#define WINERROR_CHECK_RETURN_HRESULT(err, fmt, ...) if (err != ERROR_SUCCESS) { WINERROR_PRINT(err, fmt, __VA_ARGS__); return HRESULT_FROM_WIN32(err); }
// get and format win32 error, print formatted string to debugger output, and return from calling function with error converted to HRESULT
#define WINERROR_GLE_RETURN_HRESULT(fmt, ...) WINERROR_CHECK_RETURN_HRESULT(GetLastError(), fmt, __VA_ARGS__)


// close a handle, free a memory block, or release some other resource
// auto handle = UNIQUE_HANDLE(CreateFile(...), CloseHandle);
#define UNIQUE_HANDLE_DECL(ptype, deleter) std::unique_ptr<std::remove_pointer<ptype>::type, decltype(&(deleter))>
#define UNIQUE_HANDLE(value, deleter) std::unique_ptr<std::remove_pointer<decltype(value)>::type, decltype(&(deleter))>((value), &(deleter))
