// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"

bool vietscan(const wchar_t* filename);
bool engscan(const wchar_t* filename);
bool confusescan(const wchar_t* filename);

int wmain(int argc, wchar_t** argv) {
    if (argc == 3 && !wcscmp(argv[1], L"vietscan")) {
        return !vietscan(argv[2]);
    } else if (argc == 3 && !wcscmp(argv[1], L"engscan")) {
        return !engscan(argv[2]);
    } else if (argc == 3 && !wcscmp(argv[1], L"confusescan")) {
        return !confusescan(argv[2]);
    } else {
        wprintf(L"usage: wordlister <vietscan|engscan> <filename>\n");
        return 1;
    }
}
