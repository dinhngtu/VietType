// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <thread>
#include <mutex>
#include <deque>
#include "Telex.h"
#include "TelexEngine.h"

using namespace VietType::Telex;

struct FuzzWorkItem {
    int len;
    TypingStyles style;
    std::wstring_view table;
    int start;
    int end;
};

static constexpr const int skip = 1;
static constexpr const std::wstring_view table_telex = L"acdefghinoqsuwyz";
static constexpr const std::wstring_view table_telex_complicated = L"acdefghinoqsuwyz[]";
static constexpr const std::wstring_view table_vni = L"acdeghinoquy0126789";

template <size_t table_size>
static void DoFuzz(int len, TypingStyles style, const std::wstring_view& table, int start = 0, int end = table_size) {
    if (table.size() != table_size) {
        throw std::runtime_error("invalid table size");
    }
    for (int level = 0; level <= 3; level++) {
        for (int autocorrect = 0; autocorrect <= 1; autocorrect++) {
            wprintf(L"len %d level %d autocorrect %d\n", len, level, autocorrect);
            TelexConfig config;
            config.optimize_multilang = level;
            config.autocorrect = !!autocorrect;
            config.typing_style = style;
            TelexEngine e(config);
            for (int prefix = start; prefix < end; prefix++) {
                size_t max = 1;
                for (auto i = 1; i < len; i++) {
                    max *= table_size;
                }
                for (size_t i = 0; i < max; i++) {
                    e.Reset();
                    size_t cur = i;
                    e.PushChar(table[prefix]);
                    for (auto j = 1; j < len; j++) {
                        e.PushChar(table[cur % table_size]);
                        cur /= table_size;
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

static void FuzzWorker(std::deque<FuzzWorkItem>* wq, std::mutex* wq_lock) {
    while (1) {
        FuzzWorkItem wi;
        {
            std::lock_guard guard(*wq_lock);
            if (wq->empty())
                break;
            wi = wq->front();
            wq->pop_front();
        }
        if (wi.table.size() == table_telex.size()) {
            DoFuzz<table_telex.size()>(wi.len, wi.style, wi.table, wi.start, wi.end);
        } else if (wi.table.size() == table_telex_complicated.size()) {
            DoFuzz<table_telex_complicated.size()>(wi.len, wi.style, wi.table, wi.start, wi.end);
        } else if (wi.table.size() == table_vni.size()) {
            DoFuzz<table_vni.size()>(wi.len, wi.style, wi.table, wi.start, wi.end);
        }
    }
}

bool fuzz() {
    SetPriorityClass(GetCurrentProcess(), IDLE_PRIORITY_CLASS);
    SetThreadExecutionState(ES_SYSTEM_REQUIRED | ES_CONTINUOUS);
    {
        std::deque<FuzzWorkItem> wq;
        std::mutex wq_lock;
        for (auto len = 1; len <= 5; len++) {
            wq.emplace_back(FuzzWorkItem{
                .len = len,
                .style = TypingStyles::Telex,
                .table = table_telex,
                .start = 0,
                .end = (int)table_telex.size(),
            });
            wq.emplace_back(FuzzWorkItem{
                .len = len,
                .style = TypingStyles::TelexComplicated,
                .table = table_telex_complicated,
                .start = 0,
                .end = (int)table_telex_complicated.size(),
            });
            wq.emplace_back(FuzzWorkItem{
                .len = len,
                .style = TypingStyles::Vni,
                .table = table_vni,
                .start = 0,
                .end = (int)table_vni.size(),
            });
        }
        for (auto len = 6; len <= 8; len++) {
            for (int i = 0; i < table_telex.size(); i += skip) {
                wq.emplace_back(FuzzWorkItem{
                    .len = len,
                    .style = TypingStyles::Telex,
                    .table = table_telex,
                    .start = i,
                    .end = std::min(i + skip, (int)table_telex.size()),
                });
                wq.emplace_back(FuzzWorkItem{
                    .len = len,
                    .style = TypingStyles::TelexComplicated,
                    .table = table_telex_complicated,
                    .start = i,
                    .end = std::min(i + skip, (int)table_telex_complicated.size()),
                });
                wq.emplace_back(FuzzWorkItem{
                    .len = len,
                    .style = TypingStyles::Vni,
                    .table = table_vni,
                    .start = i,
                    .end = std::min(i + skip, (int)table_vni.size()),
                });
            }
        }

        std::vector<std::jthread> workers;
        for (unsigned int i = 0; i < std::thread::hardware_concurrency(); i++) {
            workers.emplace_back(FuzzWorker, &wq, &wq_lock);
        }
    }
    SetThreadExecutionState(ES_CONTINUOUS);
    return true;
}
