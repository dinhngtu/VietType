// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"

using namespace VietType::Telex;
using namespace VietType::TestLib;

bool vietscan(const TCHAR* filename) {
    long long fsize;
    auto words = static_cast<uint16_t*>(ReadWholeFile(filename, &fsize));
    auto wend = words + fsize / sizeof(uint16_t);

    TelexConfig config;
    TelexEngine engine(config);
    for (WordListIterator w(words, wend); w != wend; w++) {
        auto word = w.to_wstring();
        if (word.empty())
            continue;
        engine.Reset();
        auto state = engine.Backconvert(word);
        switch (state) {
        case TelexStates::Valid:
            break;
        case TelexStates::Invalid:
        case TelexStates::BackconvertFailed:
            wprintf(L"%ls %d\n", word.c_str(), state);
            break;
        default:
            throw std::runtime_error("unexpected state");
        }
    }
    FreeFile(words);
    return true;
}
