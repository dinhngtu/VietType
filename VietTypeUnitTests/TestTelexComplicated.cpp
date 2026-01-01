// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestTelexComplicated", "[telex][complicated]") {
    TelexConfig config{
        .typing_style = TypingStyles::TelexComplicated,
        .oa_uy_tone1 = GENERATE(true, false),
        .accept_separate_dd = GENERATE(true, false),
        .backspaced_word_stays_invalid = GENERATE(true, false),
        .autocorrect = GENERATE(true, false),
        .optimize_multilang = static_cast<unsigned long>(GENERATE(0, 1, 2, 3)),
        .allow_abbreviations = GENERATE(true, false),
    };

    auto engine = std::unique_ptr<ITelexEngine>(TelexNew(config));

    auto TestValidWord = [&](const wchar_t* expected, const wchar_t* input) {
        UnitTests::TestValidWord(*engine, expected, input);
    };

    auto TestInvalidWord = [&](const wchar_t* expected, const wchar_t* input) {
        UnitTests::TestInvalidWord(*engine, expected, input);
    };

    auto TestPeekWord = [&](const wchar_t* expected, const wchar_t* input, TelexStates state = TelexStates::TxError) {
        UnitTests::TestPeekWord(*engine, expected, input);
        if (state != TelexStates::TxError) {
            AssertTelexStatesEqual(state, engine->GetState());
        }
    };

    SECTION("UW_ng") {
        TestValidWord(L"\x1b0ng", L"]ng");
    }
    SECTION("UWUW_ng") {
        TestInvalidWord(L"]ng", L"]]ng");
    }
    SECTION("UW_ngs") {
        TestValidWord(L"\x1ee9ng", L"]ngs");
    }
    SECTION("H_UW_ng") {
        TestValidWord(L"h\x1b0ng", L"h]ng");
    }
    SECTION("H_UW_ngs") {
        TestValidWord(L"h\x1ee9ng", L"h]ngs");
    }
    SECTION("OW_m") {
        TestValidWord(L"\x1a1m", L"[m");
    }
    SECTION("OWOW_m") {
        TestInvalidWord(L"[m", L"[[m");
    }
    SECTION("OW_ms") {
        TestValidWord(L"\x1edbm", L"[ms");
    }
    SECTION("C_OW_m") {
        TestValidWord(L"c\x1a1m", L"c[m");
    }
    SECTION("C_OW_ms") {
        TestValidWord(L"c\x1edbm", L"c[ms");
    }
    SECTION("Wng") {
        TestValidWord(L"\x1b0ng", L"wng");
    }
    SECTION("Wngs") {
        TestValidWord(L"\x1ee9ng", L"wngs");
    }
    SECTION("Uwng") {
        TestValidWord(L"\x1b0ng", L"uwng");
    }
    SECTION("Hwng") {
        TestValidWord(L"h\x1b0ng", L"hwng");
    }
    SECTION("Huwng") {
        TestValidWord(L"h\x1b0ng", L"huwng");
    }
    SECTION("Cowm") {
        TestValidWord(L"\x1a1m", L"owm");
    }
    SECTION("Huwowng") {
        TestValidWord(L"h\x1b0\x1a1ng", L"huwowng");
    }
    SECTION("Huowng") {
        TestValidWord(L"h\x1b0\x1a1ng", L"huowng");
    }
    SECTION("H_UW_OW_ng_upper") {
        TestValidWord(L"H\x1af\x1a0NG", L"H}{NG");
    }

    SECTION("TestTelexComplicatedBackspaceBracket") {
        engine->Reset();
        if (FeedWord(*engine, L"[f[za") == TelexStates::Valid) {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
        }
    }
}

} // namespace UnitTests
} // namespace VietType
