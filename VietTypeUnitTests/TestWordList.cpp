// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include <cstdlib>
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
    std::unique_ptr<wchar_t, decltype(std::free)*> words{nullptr, free};
    LONGLONG fsize = 0;

public:
    TestWordList() {
        words = {static_cast<wchar_t*>(ReadWholeFile(L"..\\..\\data\\vw39kw.txt", &fsize)), free};
    }

    TEST_METHOD (TestBackconvertWordList) {
        TelexConfig config;
        TelexEngine engine(config);
        auto wend = words.get() + fsize / sizeof(wchar_t);
        for (WordListIterator w(words.get(), wend); w != wend; w++) {
            if (!w.wlen())
                continue;
            std::wstring word(*w, w.wlen());

            engine.Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine.Backconvert(word));
            for (auto i = static_cast<int>(w.wlen()) - 1; i >= 0; i--) {
                AssertTelexStatesEqual(TelexStates::Valid, engine.Backspace());
                auto w1 = word.substr(0, i);
                auto w2 = engine.Peek();
                Assert::AreEqual(w1.c_str(), w2.c_str());
            }
        }
    }
};

} // namespace UnitTests
} // namespace VietType
