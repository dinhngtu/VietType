// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <cwchar>

bool vietscan(const TCHAR* filename);
bool engscan(const TCHAR* filename);
bool dualscan();
bool bench();

int main(int argc, char** argv) {
    if (fwide(stdout, 1) < 1)
        throw std::runtime_error("cannot set stdout to wide");
    if (argc == 3 && !_tcscmp(argv[1], TEXT("vietscan"))) {
        return !vietscan(argv[2]);
    } else if (argc == 3 && !_tcscmp(argv[1], TEXT("engscan"))) {
        return !engscan(argv[2]);
    } else if (argc == 2 && !_tcscmp(argv[1], TEXT("dualscan"))) {
        return !dualscan();
    } else if (argc == 2 && !_tcscmp(argv[1], TEXT("bench"))) {
        return !bench();
    } else {
        wprintf(L"usage: \n"
                L"    wordlister <vietscan|engscan> <filename>\n"
                L"    wordlister dualscan\n"
                L"    wordlister bench\n");
        return 1;
    }
}
