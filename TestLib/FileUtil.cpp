// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include <cstdlib>
#include <stdexcept>
#include <system_error>
#include "FileUtil.hpp"

namespace VietType {
namespace TestLib {

#ifdef WIN32
#include <Windows.h>

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

void* ReadWholeFile(const TCHAR* filename, _Out_ long long* size) {
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

void FreeFile(void* file) {
    free(file);
}

#else
#include <fcntl.h>
#include <unistd.h>

static ssize_t readall(int fd, void* buf, size_t count) {
    auto _buf = static_cast<char*>(buf);
    ssize_t rem = count;
    while (rem) {
        auto ret = read(fd, _buf, rem);
        if (ret < 0)
            return ret;
        else if (ret == 0)
            break;
        _buf += ret;
        rem -= ret;
    }
    return count - rem;
}

void* ReadWholeFile(const TCHAR* filename, _Out_ long long* size) {
    auto f = open(filename, O_RDONLY);
    if (f < 0)
        throw std::system_error(errno, std::generic_category(), "open");
    ssize_t fsize = lseek(f, 0, SEEK_END);
    if (fsize < 0) {
        close(f);
        throw std::system_error(errno, std::generic_category(), "lseek");
    }
    if (fsize > (10ll << 32)) {
        close(f);
        throw std::runtime_error("file too large");
    }
    if (lseek(f, 0, SEEK_SET)) {
        close(f);
        throw std::system_error(errno, std::generic_category(), "lseek");
    }
    auto bytes = malloc(fsize);
    if (!bytes) {
        close(f);
        throw std::bad_alloc();
    }
    if (readall(f, bytes, static_cast<size_t>(fsize)) < 0) {
        free(bytes);
        close(f);
        throw std::system_error(errno, std::generic_category(), "readall");
    }
    *size = fsize;
    return bytes;
}

void FreeFile(void* file) {
    free(file);
}

#endif

} // namespace TestLib
} // namespace VietType
