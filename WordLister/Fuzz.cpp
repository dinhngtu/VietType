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
    int len = 0;
    TypingStyles style = TypingStyles::Max;
    std::wstring_view table{};
    int start = 0;
    int end = 0;
    int mode = 0;
};

static constexpr const int skip = 1;
static constexpr const std::wstring_view table_telex = L"acdefghinoqsuwyz";
static constexpr const std::wstring_view table_telex_complicated = L"acdefghinoqsuwyz[]";
static constexpr const std::wstring_view table_vni = L"acdeghinoquy0126789";

template <size_t table_size>
static void DoFuzz(const FuzzWorkItem& wi) {
    if (wi.table.size() != table_size) {
        throw std::runtime_error("invalid table size");
    }
    for (int level = 0; level <= 3; level++) {
        for (int autocorrect = 0; autocorrect <= 1; autocorrect++) {
            wprintf(
                L"len %d style %d ts %zu start %d end %d mode %d level %d autocorrect %d\n",
                wi.len,
                (int)wi.style,
                wi.table.size(),
                wi.start,
                wi.end,
                wi.mode,
                level,
                autocorrect);
            TelexConfig config;
            config.optimize_multilang = level;
            config.autocorrect = !!autocorrect;
            config.typing_style = wi.style;
            TelexEngine e(config);
            for (int prefix = wi.start; prefix < wi.end; prefix++) {
                size_t max = 1;
                for (auto i = 1; i < wi.len; i++) {
                    max *= table_size;
                }
                for (size_t i = 0; i < max; i++) {
                    e.Reset();
                    size_t cur = i;
                    e.PushChar(wi.table[prefix]);
                    for (auto j = 1; j < wi.len; j++) {
                        e.PushChar(wi.table[cur % table_size]);
                        cur /= table_size;
                    }
                    auto keyBuffer = e.RetrieveRaw();
                    if (!e.CheckInvariants()) {
                        wprintf(L"word failed: %s\n", keyBuffer.c_str());
                    }
                    if (wi.mode == 0) {
                        if (e.Commit() == TelexStates::TxError) {
                            wprintf(L"word failed commit: %s\n", keyBuffer.c_str());
                        }
                        if (!e.CheckInvariants()) {
                            wprintf(L"word failed commit invariant: %s\n", keyBuffer.c_str());
                        }
                    } else {
                        auto prevState = e.GetState();
                        if (e.Backspace() == TelexStates::TxError) {
                            wprintf(L"word failed backspace: %s\n", keyBuffer.c_str());
                        }
                        if (!e.CheckInvariantsBackspace(prevState)) {
                            wprintf(L"word failed backspace invariant: %s\n", keyBuffer.c_str());
                        }
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
            DoFuzz<table_telex.size()>(wi);
        } else if (wi.table.size() == table_telex_complicated.size()) {
            DoFuzz<table_telex_complicated.size()>(wi);
        } else if (wi.table.size() == table_vni.size()) {
            DoFuzz<table_vni.size()>(wi);
        } else {
            throw std::runtime_error("invalid table size");
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
            for (auto mode = 0; mode <= 1; mode++) {
                wq.emplace_back(
                    FuzzWorkItem{
                        .len = len,
                        .style = TypingStyles::Telex,
                        .table = table_telex,
                        .start = 0,
                        .end = (int)table_telex.size(),
                        .mode = mode,
                    });
                wq.emplace_back(
                    FuzzWorkItem{
                        .len = len,
                        .style = TypingStyles::TelexComplicated,
                        .table = table_telex_complicated,
                        .start = 0,
                        .end = (int)table_telex_complicated.size(),
                        .mode = mode,
                    });
                wq.emplace_back(
                    FuzzWorkItem{
                        .len = len,
                        .style = TypingStyles::Vni,
                        .table = table_vni,
                        .start = 0,
                        .end = (int)table_vni.size(),
                        .mode = mode,
                    });
            }
        }
        for (auto len = 6; len <= 8; len++) {
            for (auto mode = 0; mode <= 1; mode++) {
                for (int i = 0; i < table_telex.size(); i += skip) {
                    wq.emplace_back(
                        FuzzWorkItem{
                            .len = len,
                            .style = TypingStyles::Telex,
                            .table = table_telex,
                            .start = i,
                            .end = std::min(i + skip, (int)table_telex.size()),
                            .mode = mode,
                        });
                }
                for (int i = 0; i < table_telex_complicated.size(); i += skip) {
                    wq.emplace_back(
                        FuzzWorkItem{
                            .len = len,
                            .style = TypingStyles::TelexComplicated,
                            .table = table_telex_complicated,
                            .start = i,
                            .end = std::min(i + skip, (int)table_telex_complicated.size()),
                            .mode = mode,
                        });
                }
                for (int i = 0; i < table_vni.size(); i += skip) {
                    wq.emplace_back(
                        FuzzWorkItem{
                            .len = len,
                            .style = TypingStyles::Vni,
                            .table = table_vni,
                            .start = i,
                            .end = std::min(i + skip, (int)table_vni.size()),
                            .mode = mode,
                        });
                }
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
