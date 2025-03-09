// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CLASS (TestVni) {
    const TelexConfig config{.typing_style = TypingStyles::Vni};

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
    // reset

    TEST_METHOD (TestVniEmpty) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            Assert::AreEqual(L"", e.Retrieve().c_str());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
            Assert::AreEqual(L"", e.Peek().c_str());
            Assert::AreEqual(std::size_t{0}, e.Count());
        });
    }

    // push char

    TEST_METHOD (TestVniEmptyPushCharDigits) {
        MultiConfigTester(config).Invoke([](auto& e) {
            for (wchar_t i = L'0'; i <= L'9'; i++) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Invalid, e.PushChar(i));
                auto compare = std::wstring(1, i);
                Assert::AreEqual(compare.c_str(), e.Retrieve().c_str());
            }
        });
    }

    TEST_METHOD (TestVniEmptyCommittedPushChar) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
            AssertTelexStatesEqual(TelexStates::Committed, e.PushChar('a'));
            Assert::AreEqual(L"", e.Retrieve().c_str());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
            Assert::AreEqual(L"", e.Peek().c_str());
            Assert::AreEqual(std::size_t{0}, e.Count());
        });
    }

    // uppercase

    TEST_METHOD (TestVniTypingUpAai_1) {
        TestValidWord(L"\xc2n", L"A6n");
    }

    TEST_METHOD (TestVniTypingUpDdi_1) {
        TestValidWord(L"\x110i", L"D9i");
    }

    TEST_METHOD (TestVniTypingUpAasn_1) {
        TestValidWord(L"\x1ea4n", L"A61n");
    }

    // backspace

    TEST_METHOD (TestVniEmptyBackspace) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            // must leave the class in a valid state
            Assert::AreEqual(L"", e.Retrieve().c_str());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        });
    }

    // commit

    TEST_METHOD (TestVniEmptyCommit) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
            Assert::AreEqual(L"", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestVniEmptyForceCommit) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Committed, e.ForceCommit());
            Assert::AreEqual(L"", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestVniEmptyCancel) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        });
    }

    TEST_METHOD (TestVniEmptyBackconversion) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(std::wstring()));
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        });
    }

    // word typing tests

    TEST_METHOD (TestVniTypingDdoongf) {
        TestValidWord(L"\x111\x1ed3ng", L"d9o6ng2");
    }

    TEST_METHOD (TestVniTypingAans) {
        TestValidWord(L"\x1ea5n", L"a6n1");
    }

    TEST_METHOD (TestVniTypingDdas) {
        TestValidWord(L"\x111\xe1", L"d9a1");
    }

    TEST_METHOD (TestVniTypingNhuwonxg) {
        TestValidWord(L"nh\x1b0\x1ee1ng", L"nhu7on4g");
    }

    TEST_METHOD (TestVniTypingNguiw) {
        TestValidWord(L"ng\x1b0i", L"ngui7");
    }

    TEST_METHOD (TestVniTypingThoio) {
        TestValidWord(L"th\xf4i", L"thoi6");
    }

    TEST_METHOD (TestVniTypingNua7) {
        TestValidWord(L"n\x1b0\x61", L"nua7");
    }

    TEST_METHOD (TestVniTypingNua8) {
        TestPeekWord(L"nu\x103", L"nua8");
        TestInvalidWord(L"nua8", L"nua8");
    }

    TEST_METHOD (TestVniTypingQuawms) {
        TestValidWord(L"qu\x1eafm", L"qua8m1");
    }

    TEST_METHOD (TestVniTypingQuets) {
        TestValidWord(L"qu\xe9t", L"quet1");
    }

    TEST_METHOD (TestVniTypingQuauj) {
        TestValidWord(L"qu\x1ea1u", L"quau5");
    }

    TEST_METHOD (TestVniTypingQuoj) {
        TestValidWord(L"qu\x1ecd", L"quo5");
    }

    TEST_METHOD (TestVniTypingQuitj) {
        TestValidWord(L"qu\x1ecbt", L"quit5");
    }

    TEST_METHOD (TestVniTypingQueof) {
        TestValidWord(L"qu\xe8o", L"queo2");
    }

    TEST_METHOD (TestVniTypingQuowns) {
        TestValidWord(L"qu\x1edbn", L"quo7n1");
    }

    TEST_METHOD (TestVniTypingQuwowns) {
        TestValidWord(L"q\x1b0\x1edbn", L"qu7o7n1");
    }

    TEST_METHOD (TestVniTypingQuonwx) {
        TestValidWord(L"qu\x1ee1n", L"quon74");
    }

    TEST_METHOD (TestVniTypingDduwowcj) {
        TestValidWord(L"\x111\x1b0\x1ee3\x63", L"d9u7o7c5");
    }

    TEST_METHOD (TestVniTypingNguwoif) {
        TestValidWord(L"ng\x1b0\x1eddi", L"ngu7oi2");
    }

    TEST_METHOD (TestVniTypingThuees) {
        TestValidWord(L"thu\x1ebf", L"thue61");
    }

    TEST_METHOD (TestVniTypingKhuawngs) {
        TestValidWord(L"khu\x1eafng", L"khua8ng1");
    }

    TEST_METHOD (TestVniTypingKhuawsng) {
        TestValidWord(L"khu\x1eafng", L"khua81ng");
    }

    TEST_METHOD (TestVniTypingRa) {
        TestValidWord(L"ra", L"ra");
    }

    // test variations of 'gi'

    TEST_METHOD (TestVniTypingGif) {
        TestValidWord(L"g\xec", L"gi2");
    }

    TEST_METHOD (TestVniTypingGinf) {
        TestValidWord(L"g\xecn", L"gin2");
    }

    TEST_METHOD (TestVniTypingGiuowngf) {
        TestValidWord(L"gi\x1b0\x1eddng", L"giuo7ng2");
    }

    TEST_METHOD (TestVniTypingGiowf) {
        TestValidWord(L"gi\x1edd", L"gio72");
    }

    TEST_METHOD (TestVniTypingGiuwax) {
        TestValidWord(
            L"gi\x1eef"
            "a",
            L"giu7a4");
    }

    TEST_METHOD (TestVniTypingGiux) {
        TestValidWord(L"gi\x169", L"giu4");
    }

    TEST_METHOD (TestVniTypingGiuoocj) {
        TestValidWord(
            L"giu\x1ed9"
            "c",
            L"giuo6c5");
    }

    TEST_METHOD (TestVniTypingGiemf) {
        TestValidWord(L"gi\xe8m", L"giem2");
    }

    TEST_METHOD (TestVniTypingGiee) {
        TestValidWord(L"gi\xea", L"gie6");
    }

    // test 'aua' and similar transitions

    TEST_METHOD (TestVniTypingLauar) {
        TestValidWord(L"l\x1ea9u", L"lau63");
    }

    TEST_METHOD (TestVniTypingNguayar) {
        TestValidWord(L"ngu\x1ea9y", L"nguay63");
    }

    TEST_METHOD (TestVniTypingLuuw) {
        TestValidWord(L"l\x1b0u", L"luu7");
    }

    TEST_METHOD (TestVniTypingHuouw) {
        TestValidWord(L"h\x1b0\x1a1u", L"huou7");
    }

    // irregular

    TEST_METHOD (TestVniTypingQuoiws) {
        TestValidWord(L"qu\x1edbi", L"quoi71");
    }

    TEST_METHOD (TestVniTypingDawks) {
        TestValidWord(L"\x111\x1eafk", L"d9a8k1");
    }

    // peek tests

    TEST_METHOD (TestVniPeekDd) {
        TestPeekWord(L"\x111", L"d9");
    }

    TEST_METHOD (TestVniPeekDdd) {
        TestPeekWord(L"d99", L"d99");
    }

    TEST_METHOD (TestVniPeekAd) {
        TestPeekWord(L"ad", L"ad");
    }

    TEST_METHOD (TestVniPeekQuaw) {
        TestPeekWord(L"qu\x103", L"qua8");
    }

    // used to cause a crash
    TEST_METHOD (TestVniPeekZ) {
        TestPeekWord(L"z", L"z");
    }

    TEST_METHOD (TestVniPeekCarc) {
        TestPeekWord(L"ca3c", L"ca3c");
    }

    // test peek key ordering

    TEST_METHOD (TestVniPeekCace) {
        TestPeekWord(L"cace", L"cace");
    }

    // peek shouldn't crash if tone position is not found
    TEST_METHOD (TestVniPeekNhaeng) {
        TestPeekWord(L"nhaeng", L"nhaeng");
    }

    TEST_METHOD (TestVniPeekDand) {
        auto config1 = config;
        config1.accept_separate_dd = true;
        MultiConfigTester(config1).Invoke([](auto& e) {
            FeedWord(e, L"dan9");
            Assert::AreEqual(
                L"\x111"
                "an",
                e.Peek().c_str());
        });
    }

    // double key tests

    TEST_METHOD (TestVniDoubleKeyXuaaan) {
        TestInvalidWord(L"xua6n", L"xua66n");
    }

    TEST_METHOD (TestVniDoubleKeyIis) {
        TestInvalidWord(L"iis", L"iis");
    }

    TEST_METHOD (TestVniDoubleKeyThooongf) {
        TestValidWord(L"tho\xf2ng", L"tho66ng2");
    }

    TEST_METHOD (TestVniDoubleKeyThuongz) {
        TestInvalidWord(L"thuong0", L"thuong0");
    }

    TEST_METHOD (TestVniNhuwox) {
        TestInvalidWord(L"nhu7o4", L"nhu7o4");
    }

    TEST_METHOD (TestVniToool) {
        TestInvalidWord(L"to6l", L"to66l");
    }

    TEST_METHOD (TestVniCuwowwcj) {
        TestInvalidWord(L"cu7o7c5", L"cu7o77c5");
    }

    TEST_METHOD (TestVniTypingQuwowwns) {
        TestInvalidWord(L"qu7o7n1", L"qu7o77n1");
    }

    TEST_METHOD (TestVniTypingCaasy) {
        TestValidWord(L"c\x1ea5y", L"ca61y");
    }

    // caps

    TEST_METHOD (TestVniCapsPeekD) {
        TestPeekWord(L"D", L"D");
    }

    TEST_METHOD (TestVniCapsXuOwngf) {
        TestValidWord(L"X\x1b0\x1edcng", L"XuO7ng2");
    }

    TEST_METHOD (TestVniCapsXuOnWfWx) {
        TestInvalidWord(L"XuOn7f74", L"XuOn7f74");
    }

    TEST_METHOD (TestVniAllCapsNguoiwf) {
        TestValidWord(L"NG\x1af\x1edcI", L"NGUOI72");
    }

    // backspace tests

    TEST_METHOD (TestVniBackspaceDdoongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"d9o6ng2");
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"\x111\x1ed3", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"\x111", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceLeeen) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"le66n");
            Assert::AreEqual(L"le6n", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
            Assert::AreEqual(L"le6", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceHuowng) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"huo7ng");
            Assert::AreEqual(L"h\x1b0\x1a1ng", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"h\x1b0\x1a1n", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"hu\x1a1", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"hu", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceAoas) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"ao61");
            Assert::AreEqual(L"ao61", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"ao6", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceHeei) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"he6i");
            Assert::AreEqual(L"h\xeai", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"h\xea", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceOwa) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"o7a");
            Assert::AreEqual(
                L"\x1a1"
                "a",
                e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"\x1a1", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceRuowi) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"ruo7i");
            Assert::AreEqual(L"r\x1b0\x1a1i", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"ru\x1a1", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceQuee) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"que6");
            Assert::AreEqual(L"qu\xea", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"qu", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceQuys) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"quy1");
            Assert::AreEqual(L"qu\xfd", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"qu", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceHieef) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"hie62");
            Assert::AreEqual(L"hi\x1ec1", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"hi", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceGifg) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"gi2g");
            Assert::AreEqual(L"g\xecg", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"g\xec", e.Peek().c_str());
        });
    }

    // test tone and w movements

    TEST_METHOD (TestVniBackspaceCuwsoc) {
        MultiConfigTester(config, 0, 2).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cu71oc"));
            Assert::AreEqual(
                L"c\x1b0\x1edb"
                "c",
                e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"c\x1b0\x1edb", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            // since the tone is entered with the "uw", it should stay there
            Assert::AreEqual(L"c\x1ee9", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceCuwocs) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cu7oc1"));
            Assert::AreEqual(
                L"c\x1b0\x1edb"
                "c",
                e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"c\x1b0\x1edb", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"c\x1b0", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceCuocws) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuoc71"));
            Assert::AreEqual(
                L"c\x1b0\x1edb"
                "c",
                e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"cu\x1edb", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"cu", e.Peek().c_str());
        });
    }

    // test backconversions

    TEST_METHOD (TestVniBackconversionDdoongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"\x111\x1ed3ng"));
            Assert::AreEqual(L"\x111\x1ed3ng", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionThees) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"TH\x1ebe"));
            Assert::AreEqual(L"TH\x1ebe", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionDdoonfCtrlW) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111\x1ed3n7"));
            Assert::AreEqual(L"\x111\x1ed3n7", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
            e.Cancel();
            Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
            Assert::AreEqual(L"\x111\x1ed3n", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionDdCtrlW) {
        MultiConfigTester(config, 0, 3, false).Invoke([](auto& e) {
            AssertTelexStatesEqual(
                TelexStates::BackconvertFailed,
                e.Backconvert(L"\x111"
                              "7"));
            e.Backspace();
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
        });
    }

    TEST_METHOD (TestVniBackconversionThooongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"tho\xf2ng"));
            Assert::AreEqual(L"tho\xf2ng", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"tho\xf2n", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionThooongfUpper) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"THO\xd2NG"));
            Assert::AreEqual(L"THO\xd2NG", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionXoooong) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"x\xf4\xf4ng"));
            Assert::AreEqual(L"x\xf4\xf4ng", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionXo_oong) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"xo\xf4ng"));
            Assert::AreEqual(L"xo\xf4ng", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backspace());
            Assert::AreEqual(L"xo\xf4n", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backspace());
            Assert::AreEqual(L"xo\xf4", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"xo", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionCaays) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"c\x1ea5y"));
            e.Commit();
            Assert::AreEqual(L"c\x1ea5y", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestVniBackconversionQuaays) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"qu\x1ea5y"));
            e.Commit();
            Assert::AreEqual(L"qu\x1ea5y", e.Retrieve().c_str());
        });
    }

    // test oa/oe/uy

    TEST_METHOD (TestVniTypingOaUy) {
        TestValidWord(L"ho\xe0", L"hoa2");
        TestValidWord(L"ho\xe8", L"hoe2");
        TestValidWord(L"lu\x1ef5", L"luy5");
    }

    TEST_METHOD (TestVniTypingOaUyOff) {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        MultiConfigTester(config1).Invoke([](auto& e) {
            VietType::UnitTests::TestValidWord(
                e,
                L"h\xf2"
                "a",
                L"hoa2");
            VietType::UnitTests::TestValidWord(
                e,
                L"h\xf2"
                "e",
                L"hoe2");
            VietType::UnitTests::TestValidWord(e, L"l\x1ee5y", L"luy5");
        });
    }

    // test dd accept

    TEST_METHOD (TestVniValidDodongf) {
        TestValidWord(L"\x111\x1ed3ng", L"do96ng2");
    }

    TEST_METHOD (TestVniInvalidDodongf) {
        auto config1 = config;
        config1.accept_separate_dd = false;
        MultiConfigTester(config1).Invoke(
            [](auto& e) { VietType::UnitTests::TestInvalidWord(e, L"dodongf", L"dodongf"); });
    }

    TEST_METHOD (TestVniConfigOaUy) {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        MultiConfigTester(config1).Invoke([](auto& e) { VietType::UnitTests::TestValidWord(e, L"to\xe0n", L"toan2"); });
    }

    // test doublekey backspace

    TEST_METHOD (TestVniBackspaceMooo) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"mo66");
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"mo", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestVniBackspaceMooof) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"mo662");
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"mo", e.Peek().c_str());
        });
    }
};

} // namespace UnitTests
} // namespace VietType
