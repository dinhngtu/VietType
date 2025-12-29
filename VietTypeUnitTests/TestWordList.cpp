// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include <memory>
#include <filesystem>
#include "Telex.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"
#include "Util.h"
#include "TelexEngine.h"

using namespace VietType::Telex;
using namespace VietType::TestLib;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestWordList", "[wordlist]") {
    int64_t fsize = 0;
    std::filesystem::path path = std::filesystem::path("../data/vw39kw.txt");

    if (sizeof(wchar_t) != 2)
        SKIP("VietTypeUnitTests assumes 16-bit wchar_t");

    auto words_ptr = std::unique_ptr<void, decltype(FreeFile)*>{ReadWholeFile(path, &fsize), FreeFile};
    auto words = static_cast<const wchar_t*>(words_ptr.get());
    auto wend = words + fsize / sizeof(wchar_t);

    SECTION("TestBackconvertWordList") {
        TelexConfig config1{};
        config1.oa_uy_tone1 = false;
        TelexEngine engine1(config1);

        TelexConfig config2{};
        config2.oa_uy_tone1 = true;
        TelexEngine engine2(config2);

        for (WordListIterator w(words, wend); w != wend; w++) {
            if (!w.wlen())
                continue;
            std::wstring word(*w, w.wlen());

            engine1.Reset();
            engine2.Reset();

            engine1.Backconvert(word);
            engine2.Backconvert(word);

            auto p1 = engine1.Peek();
            auto p2 = engine2.Peek();
            CHECK((word == p1 || word == p2));

            engine1.Commit();
            engine2.Commit();

            auto c1 = engine1.Retrieve();
            auto c2 = engine2.Retrieve();
            CHECK((word == c1 || word == c2));
        }
    }
}

} // namespace UnitTests
} // namespace VietType
