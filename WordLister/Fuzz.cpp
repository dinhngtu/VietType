// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <thread>
#include "Telex.h"
#include "TelexEngine.h"

using namespace VietType::Telex;

static constexpr const int skip = 4;
static constexpr const std::wstring_view table = L"abcdefghijklmnopqrstuvwxyzACDEFGHJKLMNOPQRSTUVWXYZ0123456789[]{}";

static void DoFuzz(int len, int start = 0, int end = table.size()) {
    for (int level = 0; level <= 3; level++) {
        for (int autocorrect = 0; autocorrect <= 1; autocorrect++) {
            for (int style = 0; style < (int)TypingStyles::Max; style++) {
                wprintf(L"len %d level %d autocorrect %d\n", len, level, autocorrect);
                TelexConfig config;
                config.optimize_multilang = level;
                config.autocorrect = !!autocorrect;
                config.typing_style = (TypingStyles)style;
                TelexEngine e(config);
                for (int prefix = start; prefix < end; prefix++) {
                    size_t max = 1;
                    for (auto i = 1; i < len; i++) {
                        max *= table.size();
                    }
                    for (size_t i = 0; i < max; i++) {
                        e.Reset();
                        size_t cur = i;
                        e.PushChar(table[prefix]);
                        for (auto j = 1; j < len; j++) {
                            e.PushChar(table[cur % table.size()]);
                            cur /= table.size();
                        }
                        if (!e.CheckInvariants()) {
                            wprintf(L"word failed: %s\n", e.RetrieveRaw().c_str());
                        }
                        e.Commit();
                        if (!e.CheckInvariants()) {
                            wprintf(L"word failed commit: %s\n", e.RetrieveRaw().c_str());
                        }
                    }
                }
            }
        }
    }
}

static void DoFuzzThreaded(int len) {
    std::vector<std::jthread> workers;
    for (int i = 0; i < table.size(); i += skip) {
        workers.emplace_back(DoFuzz, len, i, std::min(i + skip, (int)table.size()));
    }
}

bool fuzz() {
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
    for (auto len = 1; len <= 3; len++) {
        DoFuzz(len);
    }
    for (auto len = 4; len <= 7; len++) {
        DoFuzzThreaded(len);
    }
    return true;
}
