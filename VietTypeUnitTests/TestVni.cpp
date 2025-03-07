// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CLASS (TestVni) {
    const TelexConfig config{.vni = true};

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

    TEST_METHOD (TestEmpty) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            Assert::AreEqual(L"", e.Retrieve().c_str());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
            Assert::AreEqual(L"", e.Peek().c_str());
            Assert::AreEqual(std::size_t{0}, e.Count());
        });
    }

    // push char

    TEST_METHOD (TestEmptyPushCharDigits) {
        MultiConfigTester(config).Invoke([](auto& e) {
            for (wchar_t i = L'0'; i <= L'9'; i++) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Invalid, e.PushChar(i));
                auto compare = std::wstring(1, i);
                Assert::AreEqual(compare.c_str(), e.Retrieve().c_str());
            }
        });
    }

    TEST_METHOD (TestEmptyCommittedPushChar) {
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

    TEST_METHOD (TestTypingUpAai_1) {
        TestValidWord(L"\xc2n", L"A6n");
    }

    TEST_METHOD (TestTypingUpDdi_1) {
        TestValidWord(L"\x110i", L"D9i");
    }

    TEST_METHOD (TestTypingUpAasn_1) {
        TestValidWord(L"\x1ea4n", L"A61n");
    }

    // backspace

    TEST_METHOD (TestEmptyBackspace) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            // must leave the class in a valid state
            Assert::AreEqual(L"", e.Retrieve().c_str());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        });
    }

    // commit

    TEST_METHOD (TestEmptyCommit) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
            Assert::AreEqual(L"", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestEmptyForceCommit) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Committed, e.ForceCommit());
            Assert::AreEqual(L"", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestEmptyCancel) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        });
    }

    TEST_METHOD (TestEmptyBackconversion) {
        MultiConfigTester(config).Invoke([](auto& e) {
            e.Reset();
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(std::wstring()));
            Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        });
    }

    // word typing tests

    TEST_METHOD (TestTypingDdoongf) {
        TestValidWord(L"\x111\x1ed3ng", L"d9o6ng2");
    }

    TEST_METHOD (TestTypingAans) {
        TestValidWord(L"\x1ea5n", L"a6n1");
    }

    TEST_METHOD (TestTypingDdas) {
        TestValidWord(L"\x111\xe1", L"d9a1");
    }

    TEST_METHOD (TestTypingNhuwonxg) {
        TestValidWord(L"nh\x1b0\x1ee1ng", L"nhu7on4g");
    }

    TEST_METHOD (TestTypingNguiw) {
        TestValidWord(L"ng\x1b0i", L"ngui7");
    }

    TEST_METHOD (TestTypingThoio) {
        TestValidWord(L"th\xf4i", L"thoi6");
    }

    TEST_METHOD (TestTypingNuaw) {
        TestValidWord(
            L"n\x1b0"
            "a",
            L"nua7");
    }

    TEST_METHOD (TestTypingQuawms) {
        TestValidWord(L"qu\x1eafm", L"qua7m1");
    }

    TEST_METHOD (TestTypingQuets) {
        TestValidWord(L"qu\xe9t", L"quet1");
    }

    TEST_METHOD (TestTypingQuauj) {
        TestValidWord(L"qu\x1ea1u", L"quau5");
    }

    TEST_METHOD (TestTypingQuoj) {
        TestValidWord(L"qu\x1ecd", L"quo5");
    }

    TEST_METHOD (TestTypingQuitj) {
        TestValidWord(L"qu\x1ecbt", L"quit5");
    }

    TEST_METHOD (TestTypingQueof) {
        TestValidWord(L"qu\xe8o", L"queo2");
    }

    TEST_METHOD (TestTypingQuowns) {
        TestValidWord(L"qu\x1edbn", L"quo7n1");
    }

    TEST_METHOD (TestTypingQuwowns) {
        TestValidWord(L"q\x1b0\x1edbn", L"qu7o7n1");
    }

    TEST_METHOD (TestTypingQuonwx) {
        TestValidWord(L"qu\x1ee1n", L"quon74");
    }

    TEST_METHOD (TestTypingDduwowcj) {
        TestValidWord(L"\x111\x1b0\x1ee3\x63", L"d9u7o7c5");
    }

    TEST_METHOD (TestTypingNguwoif) {
        TestValidWord(L"ng\x1b0\x1eddi", L"ngu7oi2");
    }

    TEST_METHOD (TestTypingThuees) {
        TestValidWord(L"thu\x1ebf", L"thue61");
    }

    TEST_METHOD (TestTypingKhuawngs) {
        TestValidWord(L"khu\x1eafng", L"khua7ng1");
    }

    TEST_METHOD (TestTypingKhuawsng) {
        TestValidWord(L"khu\x1eafng", L"khua71ng");
    }

    TEST_METHOD (TestTypingRa) {
        TestValidWord(L"ra", L"ra");
    }

    // test variations of 'gi'

    TEST_METHOD (TestTypingGif) {
        TestValidWord(L"g\xec", L"gi2");
    }

    TEST_METHOD (TestTypingGinf) {
        TestValidWord(L"g\xecn", L"gin2");
    }

    TEST_METHOD (TestTypingGiuowngf) {
        TestValidWord(L"gi\x1b0\x1eddng", L"giuo7ng2");
    }

    TEST_METHOD (TestTypingGiowf) {
        TestValidWord(L"gi\x1edd", L"gio72");
    }

    TEST_METHOD (TestTypingGiuwax) {
        TestValidWord(
            L"gi\x1eef"
            "a",
            L"giu7a4");
    }

    TEST_METHOD (TestTypingGiux) {
        TestValidWord(L"gi\x169", L"giu4");
    }

    TEST_METHOD (TestTypingGiuoocj) {
        TestValidWord(
            L"giu\x1ed9"
            "c",
            L"giuo6c5");
    }

    TEST_METHOD (TestTypingGiemf) {
        TestValidWord(L"gi\xe8m", L"giem2");
    }

    TEST_METHOD (TestTypingGiee) {
        TestValidWord(L"gi\xea", L"gie6");
    }

    // test 'aua' and similar transitions

    TEST_METHOD (TestTypingLauar) {
        TestValidWord(L"l\x1ea9u", L"lau63");
    }

    TEST_METHOD (TestTypingNguayar) {
        TestValidWord(L"ngu\x1ea9y", L"nguay63");
    }

    TEST_METHOD (TestTypingLuuw) {
        TestValidWord(L"l\x1b0u", L"luu7");
    }

    TEST_METHOD (TestTypingHuouw) {
        TestValidWord(L"h\x1b0\x1a1u", L"huou7");
    }

    // irregular

    TEST_METHOD (TestTypingQuoiws) {
        TestValidWord(L"qu\x1edbi", L"quoi71");
    }

    TEST_METHOD (TestTypingDawks) {
        TestValidWord(L"\x111\x1eafk", L"d9a7k1");
    }

    // peek tests

    TEST_METHOD (TestPeekDd) {
        TestPeekWord(L"\x111", L"d9");
    }

    TEST_METHOD (TestPeekDdd) {
        TestPeekWord(L"d99", L"d99");
    }

    TEST_METHOD (TestPeekAd) {
        TestPeekWord(L"ad", L"ad");
    }

    TEST_METHOD (TestPeekQuaw) {
        TestPeekWord(L"qu\x103", L"qua7");
    }

    // used to cause a crash
    TEST_METHOD (TestPeekZ) {
        TestPeekWord(L"z", L"z");
    }

    TEST_METHOD (TestPeekCarc) {
        TestPeekWord(L"ca3c", L"ca3c");
    }

    // test peek key ordering

    TEST_METHOD (TestPeekCace) {
        TestPeekWord(L"cace", L"cace");
    }

    // peek shouldn't crash if tone position is not found
    TEST_METHOD (TestPeekNhaeng) {
        TestPeekWord(L"nhaeng", L"nhaeng");
    }

    TEST_METHOD (TestPeekDand) {
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

    TEST_METHOD (TestDoubleKeyXuaaan) {
        TestInvalidWord(L"xua6n", L"xua66n");
    }

    TEST_METHOD (TestDoubleKeyIis) {
        TestInvalidWord(L"iis", L"iis");
    }

    TEST_METHOD (TestDoubleKeyThooongf) {
        TestValidWord(L"tho\xf2ng", L"tho66ng2");
    }

    TEST_METHOD (TestDoubleKeyThuongz) {
        TestInvalidWord(L"thuong0", L"thuong0");
    }

    TEST_METHOD (TestNhuwox) {
        TestInvalidWord(L"nhu7o4", L"nhu7o4");
    }

    TEST_METHOD (TestToool) {
        TestInvalidWord(L"to6l", L"to66l");
    }

    TEST_METHOD (TestCuwowwcj) {
        TestInvalidWord(L"cu7o7c5", L"cu7o77c5");
    }

    TEST_METHOD (TestTypingQuwowwns) {
        TestInvalidWord(L"qu7o77n1", L"qu7o77n1");
    }

    TEST_METHOD (TestTypingCaasy) {
        TestValidWord(L"c\x1ea5y", L"ca61y");
    }

    /*
    // caps

    TEST_METHOD (TestCapsPeekD) {
        TestPeekWord(L"D", L"D");
    }

    TEST_METHOD (TestCapsXuOwngf) {
        TestValidWord(L"X\x1b0\x1edcng", L"XuOwngf");
    }

    TEST_METHOD (TestCapsXuOnWfWx) {
        TestInvalidWord(L"XuOnWfWx", L"XuOnWfWx");
    }

    TEST_METHOD (TestAllCapsNguoiwf) {
        TestValidWord(L"NG\x1af\x1edcI", L"NGUOIWF");
    }

    // caps repeat

    TEST_METHOD (TestCapsOSS) {
        MultiConfigTester(config, 0, 1).Invoke([](auto& e) { VietType::UnitTests::TestPeekWord(e, L"OS", L"OSS"); });
    }

    TEST_METHOD (TestCapsAAA) {
        TestInvalidWord(L"AA", L"AAA");
    }

    TEST_METHOD (TestCapsOWW) {
        TestInvalidWord(L"OW", L"OWW");
    }

    TEST_METHOD (TestCapsGIFF) {
        MultiConfigTester(config, 0, 1).Invoke([](auto& e) { VietType::UnitTests::TestPeekWord(e, L"GIF", L"GIFF"); });
    }

    TEST_METHOD (TestCapsGISS) {
        MultiConfigTester(config, 0, 1).Invoke([](auto& e) { VietType::UnitTests::TestPeekWord(e, L"GIS", L"GISS"); });
    }

    // backspace tests

    TEST_METHOD (TestBackspaceDdoongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"ddoongf");
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

    TEST_METHOD (TestBackspaceLeeen) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"leeen");
            Assert::AreEqual(L"leen", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
            Assert::AreEqual(L"lee", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceHuowng) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"huowng");
            Assert::AreEqual(L"h\x1b0\x1a1ng", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"h\x1b0\x1a1n", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"hu\x1a1", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"hu", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceAoas) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"aoas");
            Assert::AreEqual(L"aoas", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"aoa", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceHeei) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"heei");
            Assert::AreEqual(L"h\xeai", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"h\xea", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceOwa) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"owa");
            Assert::AreEqual(
                L"\x1a1"
                "a",
                e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"\x1a1", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceRuowi) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"ruowi");
            Assert::AreEqual(L"r\x1b0\x1a1i", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"ru\x1a1", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceQuee) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"quee");
            Assert::AreEqual(L"qu\xea", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"qu", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceQuys) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"quys");
            Assert::AreEqual(L"qu\xfd", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"qu", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceHieef) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"hieef");
            Assert::AreEqual(L"hi\x1ec1", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"hi", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceGifg) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"gifg");
            Assert::AreEqual(L"g\xecg", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"g\xec", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceXooong) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"xooong"));
            Assert::AreEqual(L"xoong", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"xoon", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"xoo", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceThooongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"thooongf"));
            Assert::AreEqual(L"tho\xf2ng", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"tho\xf2n", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"tho\xf2", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceAssi) {
        auto config1 = config;
        config1.backspaced_word_stays_invalid = false;
        MultiConfigTester(config1, 0, 1).Invoke([](auto& e) {
            FeedWord(e, L"asssi");
            Assert::AreEqual(L"assi", e.Peek().c_str());
            e.Backspace();
            Assert::AreEqual(L"ass", e.Peek().c_str());
        });
    }

    // test tone and w movements

    TEST_METHOD (TestBackspaceCuwsoc) {
        MultiConfigTester(config, 0, 2).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuwsoc"));
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

    TEST_METHOD (TestBackspaceCuwocs) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuwocs"));
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

    TEST_METHOD (TestBackspaceCuocws) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuocws"));
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

    TEST_METHOD (TestBackconversionDdoongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"\x111\x1ed3ng"));
            Assert::AreEqual(L"\x111\x1ed3ng", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionSystem) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Invalid, e.Backconvert(L"system"));
            Assert::AreEqual(L"system", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionThees) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"TH\x1ebe"));
            Assert::AreEqual(L"TH\x1ebe", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionVirus) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Invalid, e.Backconvert(L"virus"));
            Assert::AreEqual(L"virus", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionDdoonfCtrlW) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111\x1ed3nw"));
            Assert::AreEqual(L"\x111\x1ed3nw", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
            e.Cancel();
            Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
            Assert::AreEqual(L"\x111\x1ed3n", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestBackconversionDdCtrlW) {
        MultiConfigTester(config, 0, 3, false).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111w"));
            e.Backspace();
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
        });
    }

    TEST_METHOD (TestBackconversionXooong) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"xoong"));
            Assert::AreEqual(L"xoong", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"xoon", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionXooongUpper) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"XOONG"));
            Assert::AreEqual(L"XOONG", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionThooongf) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"tho\xf2ng"));
            Assert::AreEqual(L"tho\xf2ng", e.Peek().c_str());
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"tho\xf2n", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionThooongfUpper) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"THO\xd2NG"));
            Assert::AreEqual(L"THO\xd2NG", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionXoooong) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"x\xf4\xf4ng"));
            Assert::AreEqual(L"x\xf4\xf4ng", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackconversionXo_oong) {
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

    TEST_METHOD (TestBackconversionCaays) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"c\x1ea5y"));
            e.Commit();
            Assert::AreEqual(L"c\x1ea5y", e.Retrieve().c_str());
        });
    }

    TEST_METHOD (TestBackconversionQuaays) {
        MultiConfigTester(config).Invoke([](auto& e) {
            AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"qu\x1ea5y"));
            e.Commit();
            Assert::AreEqual(L"qu\x1ea5y", e.Retrieve().c_str());
        });
    }

    // test oa/oe/uy

    TEST_METHOD (TestTypingOaUy) {
        TestValidWord(L"ho\xe0", L"hoaf");
        TestValidWord(L"ho\xe8", L"hoef");
        TestValidWord(L"lu\x1ef5", L"luyj");
    }

    TEST_METHOD (TestTypingOaUyOff) {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        MultiConfigTester(config1).Invoke([](auto& e) {
            VietType::UnitTests::TestValidWord(
                e,
                L"h\xf2"
                "a",
                L"hoaf");
            VietType::UnitTests::TestValidWord(
                e,
                L"h\xf2"
                "e",
                L"hoef");
            VietType::UnitTests::TestValidWord(e, L"l\x1ee5y", L"luyj");
        });
    }

    // test dd accept

    TEST_METHOD (TestValidDodongf) {
        TestValidWord(L"\x111\x1ed3ng", L"dodongf");
    }

    TEST_METHOD (TestInvalidDodongf) {
        auto config1 = config;
        config1.accept_separate_dd = false;
        MultiConfigTester(config1).Invoke(
            [](auto& e) { VietType::UnitTests::TestInvalidWord(e, L"dodongf", L"dodongf"); });
    }

    TEST_METHOD (TestConfigOaUy) {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        MultiConfigTester(config1).Invoke([](auto& e) { VietType::UnitTests::TestValidWord(e, L"to\xe0n", L"toanf"); });
    }

    // test multilang optimizations

    TEST_METHOD (TestMultilangVirus) {
        auto config1 = config;
        config1.optimize_multilang = 1;
        std::unique_ptr<ITelexEngine> e(TelexNew(config1));
        VietType::UnitTests::TestInvalidWord(*e, L"virus", L"virus");
    }

    TEST_METHOD (TestMultilangDense) {
        auto config1 = config;
        config1.optimize_multilang = 2;
        std::unique_ptr<ITelexEngine> e(TelexNew(config1));
        VietType::UnitTests::TestInvalidWord(*e, L"dense", L"dense");
    }

    TEST_METHOD (TestMultilangDefe) {
        auto config1 = config;
        config1.optimize_multilang = 3;
        std::unique_ptr<ITelexEngine> e(TelexNew(config1));
        AssertTelexStatesEqual(TelexStates::Invalid, FeedWord(*e, L"defe"));
    }

    TEST_METHOD (TestMultilangVirusUpper) {
        auto config1 = config;
        config1.optimize_multilang = 1;
        std::unique_ptr<ITelexEngine> e(TelexNew(config1));
        VietType::UnitTests::TestInvalidWord(*e, L"VIRUS", L"VIRUS");
    }

    TEST_METHOD (TestMultilangDenseUpper) {
        auto config1 = config;
        config1.optimize_multilang = 2;
        std::unique_ptr<ITelexEngine> e(TelexNew(config1));
        VietType::UnitTests::TestInvalidWord(*e, L"DENSE", L"DENSE");
    }

    // test doublekey backspace

    TEST_METHOD (TestBackspaceMooo) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"mooo");
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"mo", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestBackspaceMooof) {
        MultiConfigTester(config).Invoke([](auto& e) {
            FeedWord(e, L"mooof");
            AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
            Assert::AreEqual(L"mo", e.Peek().c_str());
        });
    }

    TEST_METHOD (TestAutocorrectHwuogn) {
        auto config1 = config;
        config1.autocorrect = true;
        MultiConfigTester(config1, 0, 3, false).Invoke([](auto& e) {
            FeedWord(e, L"hwuogn");
            AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
            Assert::AreEqual(L"h\x1b0\x1a1ng", e.Retrieve().c_str());
        });
    }
    */
};

} // namespace UnitTests
} // namespace VietType
