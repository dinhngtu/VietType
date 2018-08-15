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

#include "Version.h"

namespace VietType {
namespace Version {

static std::unique_ptr<BYTE[]> GetVersionData() {
    std::array<WCHAR, MAX_PATH> dllPath;

    auto dllPathLength = GetModuleFileName(Globals::DllInstance, &dllPath[0], MAX_PATH);
    if (dllPathLength == 0) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetModuleFileName failed");
        return std::unique_ptr<BYTE[]>(nullptr);
    }
    if (dllPathLength >= MAX_PATH) {
        dllPathLength--;
    }
    dllPath[dllPathLength] = 0;

    DWORD h = 0;
    auto infoSize = GetFileVersionInfoSize(&dllPath[0], &h);
    if (infoSize == 0) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetFileVersionInfoSize failed");
        return std::unique_ptr<BYTE[]>(nullptr);
    }

    auto versionData = std::make_unique<BYTE[]>(infoSize);
    if (!GetFileVersionInfo(&dllPath[0], NULL, infoSize, versionData.get())) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetFileVersionInfo failed");
        return std::unique_ptr<BYTE[]>(nullptr);
    }

    return versionData;
}

_Check_return_ _Success_(return) bool GetFileVersion(_Out_ WORD* major, _Out_ WORD* minor, _Out_ WORD* build, _Out_ WORD* privt) {
    auto versionData = GetVersionData();
    if (!versionData) {
        return false;
    }

    VS_FIXEDFILEINFO *fileInfo;
    UINT fileInfoSize;
    if (!VerQueryValue(versionData.get(), L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoSize)) {
        WINERROR_PRINT(GetLastError(), L"%s", L"VerQueryValue failed");
        return false;
    }

    *major = HIWORD(fileInfo->dwFileVersionMS);
    *minor = LOWORD(fileInfo->dwFileVersionMS);
    *build = HIWORD(fileInfo->dwFileVersionLS);
    *privt = LOWORD(fileInfo->dwFileVersionLS);

    return true;
}

_Check_return_ _Success_(return) bool GetProductVersion(_Out_ WORD* major, _Out_ WORD* minor, _Out_ WORD* build, _Out_ WORD* privt) {
    auto versionData = GetVersionData();
    if (!versionData) {
        return false;
    }

    VS_FIXEDFILEINFO *fileInfo;
    UINT fileInfoSize;
    if (!VerQueryValue(versionData.get(), L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoSize)) {
        WINERROR_PRINT(GetLastError(), L"%s", L"VerQueryValue failed");
        return false;
    }

    *major = HIWORD(fileInfo->dwProductVersionMS);
    *minor = LOWORD(fileInfo->dwProductVersionMS);
    *build = HIWORD(fileInfo->dwProductVersionLS);
    *privt = LOWORD(fileInfo->dwProductVersionLS);

    return true;
}

}
}
