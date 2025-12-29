// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"
#include "TelexEngine.h"

using namespace VietType::Telex;
using namespace VietType::TestLib;

enum DualScanMode {
    WlistEn2,
    WlistEnAc,
};

bool dualscan(int mode) {
    std::set<std::wstring> vwordset;
    {
        int64_t vfsize;
        auto vpath = std::filesystem::path("..") / ".." / "data" / "vw39kw.txt";
        auto vwords = static_cast<wchar_t*>(ReadWholeFile(vpath, &vfsize));
        auto vwend = vwords + vfsize / sizeof(wchar_t);
        for (WordListIterator vw(vwords, vwend); vw != vwend; vw++) {
            vwordset.insert(std::wstring(*vw, vw.wlen()));
        }
        FreeFile(vwords);
    }

    int64_t efsize;
    auto epath = std::filesystem::path("..") / ".." / "data" / "ewdsw.txt";
    auto ewords = static_cast<wchar_t*>(ReadWholeFile(epath, &efsize));
    auto ewend = ewords + efsize / sizeof(wchar_t);
    TelexConfig config;
    switch (mode) {
    case WlistEn2:
        config.optimize_multilang = 1;
        config.autocorrect = false;
        break;
    case WlistEnAc:
        config.optimize_multilang = 0;
        config.autocorrect = true;
        break;
    }
    TelexEngine engine(config);
    for (WordListIterator ew(ewords, ewend); ew != ewend; ew++) {
        std::wstring eword(*ew, ew.wlen());
        engine.Reset();
        for (auto c : eword) {
            engine.PushChar(c);
        }
        if (engine.Commit() == TelexStates::Committed) {
            auto outword = engine.Retrieve();
            switch (mode) {
            case WlistEn2:
                if (vwordset.find(outword) == vwordset.end() &&
                    std::any_of(
                        engine.GetRespos().begin(), engine.GetRespos().end(), [](auto x) { return x & ~ResposMask; })) {
                    wprintf(L"%ls\n", eword.c_str());
                }
                break;
            case WlistEnAc:
                if (engine.IsAutocorrected()) {
                    wprintf(L"%ls\n", eword.c_str());
                }
                break;
            }
        }
    }
    FreeFile(ewords);
    return true;
}
