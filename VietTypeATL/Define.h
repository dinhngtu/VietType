// SPDX-License-Identifier: GPL-3.0-only

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
#define HRESULT_CHECK_NOASSERT(hr, fmt, ...) if (FAILED(hr)) { HRESULT_PRINT(hr, fmt, __VA_ARGS__); }
// assert if HRESULT is successful; if not, print error information to debugger output
#define HRESULT_CHECK(hr, fmt, ...) if (FAILED(hr)) { assert(!FAILED(hr)); HRESULT_PRINT(hr, fmt, __VA_ARGS__); }
// assert if HRESULT is successful; if not, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN(hr, fmt, ...) if (FAILED(hr)) { assert(!FAILED(hr)); HRESULT_PRINT(hr, fmt, __VA_ARGS__); return hr; }
// assert if HRESULT is successful; if not, set *ptr to nullptr, print error information to debugger output and return from calling function
#define HRESULT_CHECK_RETURN_OUTPTR(hr, ptr, fmt, ...) if (FAILED(hr)) { assert(!FAILED(hr)); *ptr = nullptr; HRESULT_PRINT(hr, fmt, __VA_ARGS__); return hr; }

#ifdef _DEBUG
// if in debug builds, print formatted string to debugger output
#define DBG_DPRINT DPRINT
// if in debug builds, check if HRESULT is successful; if not, print error information to debugger output
#define DBG_HRESULT_CHECK HRESULT_CHECK_NOASSERT
#else
#define DBG_DPRINT(fmt, ...)
#define DBG_HRESULT_CHECK(hr, fmt, ...)
#endif

// format win32 error and print formatted string to debugger output
#define WINERROR_PRINT(err, fmt, ...) _errorprint(__FUNCTIONW__, __LINE__, err, L"%s:%d: WINERROR %lx (%s): " fmt L"\n", __VA_ARGS__)
// assert & format win32 error, print formatted string to debugger output, and return from calling function with error converted to HRESULT
#define WINERROR_CHECK_RETURN_HRESULT(err, fmt, ...) if (err != ERROR_SUCCESS) { assert(err == ERROR_SUCCESS); WINERROR_PRINT(err, fmt, __VA_ARGS__); return HRESULT_FROM_WIN32(err); }
// get and format win32 error, print formatted string to debugger output, and return from calling function with error converted to HRESULT
#define WINERROR_GLE_RETURN_HRESULT(fmt, ...) WINERROR_CHECK_RETURN_HRESULT(GetLastError(), fmt, __VA_ARGS__)


// UNIQUE_HANDLE's type
// UNIQUE_HANDLE_DECL(HANDLE, CloseHandle) handle;
#define UNIQUE_HANDLE_DECL(ptype, deleter) std::unique_ptr<std::remove_pointer<ptype>::type, decltype(&(deleter))>
// close a handle, free a memory block, or release some other resource
// auto handle = UNIQUE_HANDLE(CreateFile(...), CloseHandle);
#define UNIQUE_HANDLE(value, deleter) std::unique_ptr<std::remove_pointer<decltype(value)>::type, decltype(&(deleter))>((value), &(deleter))

#define GUID_WFORMAT L"%08lx-%04hx-%04hx-%02hhx%02hhx-%02hhx%02hhx%02hhx%02hhx%02hhx%02hhx"
#define GUID_COMPONENTS(guid) guid.Data1, guid.Data2, guid.Data3, \
guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]
