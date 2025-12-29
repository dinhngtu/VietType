// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"

bool vietscan(const wchar_t* filename);
bool engscan(const wchar_t* filename);
bool dualscan(int mode);
bool bench();
bool fuzz();

static_assert(sizeof(wchar_t) == 2, "VietTypeUnitTests assumes 16-bit wchar_t");

#ifdef _WIN32
int wmain(int argc, wchar_t** argv) {
#else
int main(int argc, char** argv) {
    (void)argc;
    (void)argv;
    return 1;
}
int wmain_shim(int argc, wchar_t** argv) {
#endif
    if (argc == 3 && !wcscmp(argv[1], L"vietscan")) {
        return !vietscan(argv[2]);
    } else if (argc == 3 && !wcscmp(argv[1], L"engscan")) {
        return !engscan(argv[2]);
    } else if (argc >= 2 && !wcscmp(argv[1], L"dualscan")) {
        int mode = 0;
        if (argc >= 3)
            mode = _wtoi(argv[2]);
        return !dualscan(mode);
    } else if (argc == 2 && !wcscmp(argv[1], L"bench")) {
        return !bench();
    } else if (argc == 2 && !wcscmp(argv[1], L"fuzz")) {
        return !fuzz();
    } else {
        wprintf(
            L"usage: \n"
            L"    wordlister <vietscan|engscan> <filename>\n"
            L"    wordlister dualscan\n"
            L"    wordlister bench\n"
            L"    wordlister fuzz\n");
        return 1;
    }
}
