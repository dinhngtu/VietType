// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <chrono>
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"

using namespace VietType::Telex;
using namespace VietType::TestLib;

#ifdef _DEBUG
#define EITERATIONS 10
#define VITERATIONS 200
#else
#define EITERATIONS 100
#define VITERATIONS 2000
#endif

bool bench() {
    {
        long long efsize;
        auto ewords = static_cast<uint16_t*>(ReadWholeFile(DATA("ewdsw.txt"), &efsize));
        auto ewend = ewords + efsize / sizeof(uint16_t);
        TelexConfig config;
        TelexEngine engine(config);
        unsigned long long count = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < EITERATIONS; i++) {
            for (WordListIterator ew(ewords, ewend); ew != ewend; ew++) {
                auto eword = ew.to_wstring();
                engine.Reset();
                for (auto c : eword) {
                    engine.PushChar(c);
                    engine.Peek();
                }
                engine.Commit();
                engine.Retrieve();
                count++;
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        wprintf(
            L"ewords total iters: %d, count = %llu, time = %llu us\n",
            EITERATIONS,
            count,
            std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
        FreeFile(ewords);
    }

    {
        long long vfsize;
        auto vwords = static_cast<uint16_t*>(ReadWholeFile(DATA("vw39kw.txt"), &vfsize));
        auto vwend = vwords + vfsize / sizeof(uint16_t);
        TelexConfig config;
        TelexEngine engine(config);
        unsigned long long count = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < VITERATIONS; i++) {
            for (WordListIterator vw(vwords, vwend); vw != vwend; vw++) {
                auto vword = vw.to_wstring();
                if (vword.empty())
                    continue;
                engine.Reset();
                engine.Backconvert(vword);
                count++;
            }
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        wprintf(
            L"vwords total iters: %d, count = %llu, time = %llu us\n",
            VITERATIONS,
            count,
            std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());
        FreeFile(vwords);
    }
    return true;
}
