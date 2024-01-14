// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include "FileUtil.hpp"

namespace VietType {
namespace TestLib {

static BOOL readall(HANDLE fd, PVOID buf, DWORD count) {
    auto _buf = static_cast<char*>(buf);
    DWORD rem = count;
    while (rem) {
        DWORD done;
        if (!ReadFile(fd, _buf, static_cast<DWORD>(rem), &done, NULL))
            return FALSE;
        else if (done == 0)
            break;
        _buf += done;
        rem -= done;
    }
    return TRUE;
}

PVOID ReadWholeFile(PCWSTR filename, _Out_ PLONGLONG size) {
    auto f = CreateFileW(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if (f == INVALID_HANDLE_VALUE)
        throw std::system_error(GetLastError(), std::system_category(), "CreateFileW");
    LARGE_INTEGER fsize;
    if (!GetFileSizeEx(f, &fsize)) {
        CloseHandle(f);
        throw std::system_error(GetLastError(), std::system_category(), "GetFileSizeEx");
    }
    if (fsize.QuadPart > (10ll << 32)) {
        CloseHandle(f);
        throw std::runtime_error("file too large");
    }
    auto bytes = malloc(fsize.QuadPart);
    if (!bytes) {
        CloseHandle(f);
        throw std::bad_alloc();
    }
    if (!readall(f, bytes, static_cast<DWORD>(fsize.QuadPart))) {
        free(bytes);
        CloseHandle(f);
        throw std::system_error(GetLastError(), std::system_category(), "ReadFile");
    }
    *size = fsize.QuadPart;
    return bytes;
}

VOID FreeFile(PVOID file) {
    free(file);
}

} // namespace TestLib
} // namespace VietType
