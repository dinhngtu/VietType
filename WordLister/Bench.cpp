// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <chrono>
#include "Telex.h"
#include "TelexEngine.h"
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
        LONGLONG efsize;
        auto ewords = static_cast<wchar_t*>(ReadWholeFile(L"..\\..\\data\\ewdsw.txt", &efsize));
        auto ewend = ewords + efsize / sizeof(wchar_t);
        TelexConfig config;
        TelexEngine engine(config);
        unsigned long long count = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < EITERATIONS; i++) {
            for (WordListIterator ew(ewords, ewend); ew != ewend; ew++) {
                std::wstring eword(*ew, ew.wlen());
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
        LONGLONG vfsize;
        auto vwords = static_cast<wchar_t*>(ReadWholeFile(L"..\\..\\data\\vw39kw.txt", &vfsize));
        auto vwend = vwords + vfsize / sizeof(wchar_t);
        TelexConfig config;
        TelexEngine engine(config);
        unsigned long long count = 0;
        auto t1 = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < VITERATIONS; i++) {
            for (WordListIterator vw(vwords, vwend); vw != vwend; vw++) {
                std::wstring vword(*vw, vw.wlen());
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
