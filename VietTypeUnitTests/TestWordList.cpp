// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <cstdlib>
#include <cstdio>
#include <memory>
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"
#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VietType::Telex;
using namespace VietType::TestLib;

namespace VietType {
namespace UnitTests {

TEST_CLASS (TestWordList) {
    std::unique_ptr<wchar_t, decltype(FreeFile)*> words{nullptr, FreeFile};
    LONGLONG fsize = 0;

public:
    TestWordList() {
        words = {static_cast<wchar_t*>(ReadWholeFile(L"..\\..\\data\\vw39kw.txt", &fsize)), FreeFile};
    }

    TEST_METHOD (TestBackconvertWordList) {
        TelexConfig config1;
        config1.oa_uy_tone1 = false;
        TelexEngine engine1(config1);

        TelexConfig config2;
        config1.oa_uy_tone1 = true;
        TelexEngine engine2(config2);

        auto wend = words.get() + fsize / sizeof(wchar_t);
        for (WordListIterator w(words.get(), wend); w != wend; w++) {
            if (!w.wlen())
                continue;
            std::wstring word(*w, w.wlen());

            engine1.Reset();
            engine2.Reset();

            engine1.Backconvert(word);
            engine2.Backconvert(word);
            /*
            if (engine1.GetState() != TelexStates::Valid || engine2.GetState() != TelexStates::Valid)
                __debugbreak();
            AssertTelexStatesEqual(TelexStates::Valid, engine1.GetState());
            AssertTelexStatesEqual(TelexStates::Valid, engine2.GetState());
            */
            auto p1 = engine1.Peek(), p2 = engine2.Peek();
            /*
            if (word != p1 && word != p2)
                __debugbreak();
            */
            Assert::IsTrue(word == p1 || word == p2);

            engine1.Commit();
            engine2.Commit();
            /*
            if (engine1.GetState() != TelexStates::Committed || engine2.GetState() != TelexStates::Committed)
                __debugbreak();
            AssertTelexStatesEqual(TelexStates::Committed, engine1.GetState());
            AssertTelexStatesEqual(TelexStates::Committed, engine2.GetState());
            */
            auto c1 = engine1.Retrieve(), c2 = engine2.Retrieve();
            /*
            if (word != c1 && word != c2)
                __debugbreak();
            */
            Assert::IsTrue(word == c1 || word == c2);
        }
    }
};

} // namespace UnitTests
} // namespace VietType
