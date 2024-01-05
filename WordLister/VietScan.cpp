#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <cstdio>
#include "stdafx.h"
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"

using namespace VietType::Telex;

bool vietscan(const wchar_t* filename) {
    LONGLONG fsize;
    auto words = static_cast<wchar_t*>(ReadWholeFile(filename, &fsize));
    auto wend = words + fsize / sizeof(wchar_t);

    TelexConfig config;
    TelexEngine engine(config);
    for (WordListIterator w(words, wend); w != wend; w++) {
        std::wstring word(*w, w.wlen());

        engine.Reset();
        auto state = engine.Backconvert(word);
        switch (state) {
        case TelexStates::Valid:
            break;
        case TelexStates::Invalid:
        case TelexStates::BackconvertFailed:
            wprintf(L"%s %d\n", word.c_str(), state);
            break;
        default:
            throw std::runtime_error("unexpected state");
        }
    }
    free(words);
    return true;
}
