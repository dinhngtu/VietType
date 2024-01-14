// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"

bool vietscan(const wchar_t* filename);
bool engscan(const wchar_t* filename);
bool dualscan();
bool bench();

int wmain(int argc, wchar_t** argv) {
    if (argc == 3 && !wcscmp(argv[1], L"vietscan")) {
        return !vietscan(argv[2]);
    } else if (argc == 3 && !wcscmp(argv[1], L"engscan")) {
        return !engscan(argv[2]);
    } else if (argc == 2 && !wcscmp(argv[1], L"dualscan")) {
        return !dualscan();
    } else if (argc == 2 && !wcscmp(argv[1], L"bench")) {
        return !bench();
    } else {
        wprintf(L"usage: \n"
                L"    wordlister <vietscan|engscan> <filename>\n"
                L"    wordlister dualscan\n"
                L"    wordlister bench\n");
        return 1;
    }
}
