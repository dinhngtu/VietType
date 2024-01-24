// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <thread>
#include "Telex.h"

using namespace VietType::Telex;

static void DoFuzz(int len, wchar_t start = 0) {
    for (int level = 0; level <= 3; level++) {
        for (int autocorrect = 0; autocorrect <= 1; autocorrect++) {
            TelexConfig config;
            config.optimize_multilang = level;
            config.autocorrect = !!autocorrect;
            TelexEngine e(config);
            size_t max = 1;
            for (auto i = start ? 1 : 0; i < len; i++) {
                max *= 26;
            }
            for (size_t i = 0; i < max; i++) {
                std::wstring word(start ? start : L'a', len);
                size_t cur = i;
                for (auto j = start ? 1 : 0; j < len; j++) {
                    word[j] = L'a' + cur % 26;
                    cur /= 26;
                }
                e.Reset();
                for (auto c : word) {
                    e.PushChar(c);
                }
                if (!e.CheckInvariants()) {
                    wprintf(L"word failed: %s\n", word.c_str());
                }
                e.Commit();
                if (!e.CheckInvariants()) {
                    wprintf(L"word failed commit: %s\n", word.c_str());
                }
                if (i % (1ull << 24) == 0) {
                    wprintf(L"done %zu words\n", i);
                }
            }
        }
    }
}

static void DoFuzzThreaded(int len) {
    std::vector<std::thread> workers;
    for (wchar_t i = L'a'; i <= L'z'; i++) {
        workers.emplace_back(DoFuzz, len, i);
    }
    for (auto& w : workers)
        w.join();
}

bool fuzz() {
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
    for (auto len = 1; len <= 4; len++) {
        wprintf(L"len %d\n", len);
        DoFuzz(len);
    }
    for (auto len = 5; len <= 7; len++) {
        wprintf(L"len %d\n", len);
        DoFuzzThreaded(len);
    }
    return true;
}
