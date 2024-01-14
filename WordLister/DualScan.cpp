// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <set>
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"
#include "TelexEngine.h"

using namespace VietType::Telex;
using namespace VietType::TestLib;

bool dualscan() {
    std::set<std::wstring> vwordset;
    {
        LONGLONG vfsize;
        auto vwords = static_cast<wchar_t*>(ReadWholeFile(L"..\\..\\data\\vw39kw.txt", &vfsize));
        auto vwend = vwords + vfsize / sizeof(wchar_t);
        for (WordListIterator vw(vwords, vwend); vw != vwend; vw++) {
            vwordset.insert(std::wstring(*vw, vw.wlen()));
        }
        FreeFile(vwords);
    }

    LONGLONG efsize;
    auto ewords = static_cast<wchar_t*>(ReadWholeFile(L"..\\..\\data\\ewdsw.txt", &efsize));
    auto ewend = ewords + efsize / sizeof(wchar_t);
    TelexConfig config;
    TelexEngine engine(config);
    for (WordListIterator ew(ewords, ewend); ew != ewend; ew++) {
        std::wstring eword(*ew, ew.wlen());
        engine.Reset();
        for (auto c : eword) {
            engine.PushChar(c);
        }
        if (engine.Commit() == TelexStates::Committed) {
            auto outword = engine.Retrieve();
            if (vwordset.find(outword) == vwordset.end() &&
                std::any_of(
                    engine.GetRespos().begin(), engine.GetRespos().end(), [](auto x) { return x & ~ResposMask; })) {
                wprintf(L"%s\n", eword.c_str());
            }
        }
    }
    FreeFile(ewords);
    return true;
}
