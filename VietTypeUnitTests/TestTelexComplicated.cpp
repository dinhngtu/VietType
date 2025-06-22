// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CLASS (TestTelexComplicated) {
    const TelexConfig config{.typing_style = TypingStyles::TelexComplicated};

    void TestValidWord(const wchar_t* expected, const wchar_t* input) const {
        MultiConfigTester(config).Invoke([=](auto& e) { VietType::UnitTests::TestValidWord(e, expected, input); });
    }

    void TestInvalidWord(const wchar_t* expected, const wchar_t* input) const {
        MultiConfigTester(config).Invoke([=](auto& e) { VietType::UnitTests::TestInvalidWord(e, expected, input); });
    }

    void TestPeekWord(const wchar_t* expected, const wchar_t* input, TelexStates state = TelexStates::TxError) const {
        MultiConfigTester(config).Invoke([=](auto& e) {
            VietType::UnitTests::TestPeekWord(e, expected, input);
            if (state != TelexStates::TxError) {
                AssertTelexStatesEqual(state, e.GetState());
            }
        });
    }

public:
    TEST_METHOD (TestTelexComplicatedUW_ng) {
        TestValidWord(L"\x1b0ng", L"[ng");
    }

    TEST_METHOD (TestTelexComplicatedUW_ngs) {
        TestValidWord(L"\x1ee9ng", L"[ngs");
    }

    TEST_METHOD (TestTelexComplicatedH_UW_ng) {
        TestValidWord(L"h\x1b0ng", L"h[ng");
    }

    TEST_METHOD (TestTelexComplicatedH_UW_ngs) {
        TestValidWord(L"h\x1ee9ng", L"h[ngs");
    }

    TEST_METHOD (TestTelexComplicatedOW_m) {
        TestValidWord(L"\x1a1m", L"]m");
    }

    TEST_METHOD (TestTelexComplicatedOW_ms) {
        TestValidWord(L"\x1edbm", L"]ms");
    }

    TEST_METHOD (TestTelexComplicatedC_OW_m) {
        TestValidWord(L"c\x1a1m", L"c]m");
    }

    TEST_METHOD (TestTelexComplicatedC_OW_ms) {
        TestValidWord(L"c\x1edbm", L"c]ms");
    }

    TEST_METHOD (TestTelexComplicatedWng) {
        TestValidWord(L"\x1b0ng", L"wng");
    }

    TEST_METHOD (TestTelexComplicatedWngs) {
        TestValidWord(L"\x1ee9ng", L"wngs");
    }
};

} // namespace UnitTests
} // namespace VietType
