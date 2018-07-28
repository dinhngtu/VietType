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

std::tuple<WORD, WORD, WORD, WORD> VietType::ReadVersionData() {
    auto versionDefault = std::make_tuple((WORD)0, (WORD)0, (WORD)0, (WORD)0);

    std::array<WCHAR, MAX_PATH> dllPath;

    auto dllPathLength = GetModuleFileName(VietType::Globals::DllInstance, &dllPath[0], MAX_PATH);
    if (dllPathLength == 0) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetModuleFileName failed");
        return versionDefault;
    }
    if (dllPathLength >= MAX_PATH) {
        dllPathLength--;
    }
    dllPath[dllPathLength] = 0;

    DWORD h = 0;
    auto infoSize = GetFileVersionInfoSize(&dllPath[0], &h);
    if (!infoSize) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetFileVersionInfoSize failed");
        return versionDefault;
    }

    auto versionData = std::make_unique<BYTE[]>(infoSize);
    if (!GetFileVersionInfo(&dllPath[0], NULL, infoSize, versionData.get())) {
        WINERROR_PRINT(GetLastError(), L"%s", L"GetFileVersionInfo failed");
        return versionDefault;
    }

    VS_FIXEDFILEINFO *fileInfo;
    UINT fileInfoSize;
    if (!VerQueryValue(versionData.get(), L"\\", reinterpret_cast<LPVOID*>(&fileInfo), &fileInfoSize)) {
        WINERROR_PRINT(GetLastError(), L"%s", L"VerQueryValue failed");
        return versionDefault;
    }

    return std::make_tuple(
        HIWORD(fileInfo->dwFileVersionMS),
        LOWORD(fileInfo->dwFileVersionMS),
        HIWORD(fileInfo->dwFileVersionLS),
        LOWORD(fileInfo->dwFileVersionLS));
}
