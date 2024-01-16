// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"
#include "TelexEngine.h"

using namespace VietType::Telex;
using namespace VietType::TestLib;

static TelexStates TestWord(TelexEngine& e, const wchar_t* input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
    return e.Commit();
}

bool engscan(const TCHAR* filename) {
    long long fsize;
    auto words = static_cast<uint16_t*>(ReadWholeFile(filename, &fsize));
    auto wend = words + fsize / sizeof(uint16_t);

    TelexConfig config;
    config.optimize_multilang = 0;
    TelexEngine engine(config);
    for (WordListIterator w(words, wend); w != wend; w++) {
        auto word = w.to_wstring();

        auto state = TestWord(engine, word.c_str());
        if (state == TelexStates::Committed && engine.GetTone() != Tones::Z) {
            const wchar_t* wordclass = L"";
            if (std::count_if(
                    engine.GetRespos().begin(), engine.GetRespos().end(), [](auto x) { return x & ResposTone; }) > 1)
                wordclass = L"DoubleTone";
            else if (!(*engine.GetRespos().rbegin() & ResposTone))
                wordclass = L"ToneNotEnd";
            wprintf(L"%ls %ls\n", word.c_str(), wordclass);
        }
    }
    FreeFile(words);
    return true;
}
