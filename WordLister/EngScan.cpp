// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"

using namespace VietType::Telex;
using namespace VietType::TestLib;

enum ResposTransitions {
    ResposExpunged = -1,
    ResposTransitionC1 = -2,
    ResposTransitionV = -3,
    ResposTransitionW = -4,
    ResposTone = -5,
};

static TelexStates TestWord(TelexEngine& e, const wchar_t* input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
    return e.Commit();
}

bool engscan(const wchar_t* filename) {
    LONGLONG fsize;
    auto words = static_cast<wchar_t*>(ReadWholeFile(filename, &fsize));
    auto wend = words + fsize / sizeof(wchar_t);

    TelexConfig config;
    TelexEngine engine(config);
    for (WordListIterator w(words, wend); w != wend; w++) {
        std::wstring word(*w, w.wlen());

        auto state = TestWord(engine, word.c_str());
        if (state == TelexStates::Committed && engine.GetTone() != Tones::Z) {
            const wchar_t* wordclass = L"";
            if (std::count(engine.GetRespos().begin(), engine.GetRespos().end(), ResposTone) > 1)
                wordclass = L"DoubleTone";
            else if (*engine.GetRespos().rbegin() != ResposTone)
                wordclass = L"ToneNotEnd";
            wprintf(L"%s %s\n", word.c_str(), wordclass);
        }
    }
    FreeFile(words);
    return true;
}
