// SPDX-License-Identifier: GPL-3.0-only

#include "stdafx.h"
#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CLASS (TestTelex) {
    const TelexConfig config{};

    void TestValidWord(const wchar_t* expected, const wchar_t* input) const {
        {
            auto config_ = config;
            config_.optimize_multilang = TelexConfig::OptimizeMultilang::Off;
            TelexEngine e(config_);
            VietType::UnitTests::TestValidWord(e, expected, input);
        }
        {
            auto config_ = config;
            config_.optimize_multilang = TelexConfig::OptimizeMultilang::On;
            TelexEngine e(config_);
            VietType::UnitTests::TestValidWord(e, expected, input);
        }
    }

    void TestInvalidWord(const wchar_t* expected, const wchar_t* input) const {
        {
            auto config_ = config;
            config_.optimize_multilang = TelexConfig::OptimizeMultilang::Off;
            TelexEngine e(config);
            VietType::UnitTests::TestInvalidWord(e, expected, input);
        }
        {
            auto config_ = config;
            config_.optimize_multilang = TelexConfig::OptimizeMultilang::On;
            TelexEngine e(config);
            VietType::UnitTests::TestInvalidWord(e, expected, input);
        }
    }

    void TestPeekWord(const wchar_t* expected, const wchar_t* input) const {
        {
            auto config_ = config;
            config_.optimize_multilang = TelexConfig::OptimizeMultilang::Off;
            TelexEngine e(config_);
            VietType::UnitTests::TestPeekWord(e, expected, input);
        }
        {
            auto config_ = config;
            config_.optimize_multilang = TelexConfig::OptimizeMultilang::On;
            TelexEngine e(config_);
            VietType::UnitTests::TestPeekWord(e, expected, input);
        }
    }

public:
    // reset

    TEST_METHOD (TestEmpty) {
        TelexEngine e(config);
        e.Reset();
        Assert::AreEqual(L"", e.Retrieve().c_str());
        Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        Assert::AreEqual(L"", e.Peek().c_str());
        Assert::AreEqual(std::size_t{0}, e.Count());
    }

    // push char

    TEST_METHOD (TestEmptyPushCharC1_1) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'b'));
        Assert::AreEqual(L"b", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyPushCharC1_2) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'c'));
        Assert::AreEqual(L"c", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyPushCharC1_3) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'd'));
        Assert::AreEqual(L"d", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyPushCharC1_4) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'r'));
        Assert::AreEqual(L"r", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyPushCharC1_5) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'x'));
        Assert::AreEqual(L"x", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyPushCharV) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'a'));
        Assert::AreEqual(L"a", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyCommittedPushChar) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
        AssertTelexStatesEqual(TelexStates::Committed, e.PushChar('a'));
        Assert::AreEqual(L"", e.Retrieve().c_str());
        Assert::AreEqual(L"", e.RetrieveRaw().c_str());
        Assert::AreEqual(L"", e.Peek().c_str());
        Assert::AreEqual(std::size_t{0}, e.Count());
    }

    // uppercase

    TEST_METHOD (TestEmptyPushCharUpC1) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'B'));
        Assert::AreEqual(L"B", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyPushCharUpV) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'A'));
        Assert::AreEqual(L"A", e.Retrieve().c_str());
    }

    TEST_METHOD (TestTypingUpAai_1) {
        TestValidWord(L"\xc2n", L"AAn");
    }

    TEST_METHOD (TestTypingUpAai_2) {
        TestValidWord(L"\xc2n", L"Aan");
    }

    TEST_METHOD (TestTypingUpDdi_1) {
        TestValidWord(L"\x110i", L"DDi");
    }

    TEST_METHOD (TestTypingUpDdi_2) {
        TestValidWord(L"\x110i", L"Ddi");
    }

    TEST_METHOD (TestTypingUpAasn_1) {
        TestValidWord(L"\x1ea4n", L"AASn");
    }

    TEST_METHOD (TestTypingUpAasn_2) {
        TestValidWord(L"\x1ea4n", L"Aasn");
    }

    TEST_METHOD (TestTypingUpAasn_3) {
        TestValidWord(L"\x1ea4n", L"AAsn");
    }

    // backspace

    TEST_METHOD (TestEmptyBackspace) {
        TelexEngine e(config);
        e.Reset();
        // must leave the class in a valid state
        Assert::AreEqual(L"", e.Retrieve().c_str());
        Assert::AreEqual(L"", e.RetrieveRaw().c_str());
    }

    // commit

    TEST_METHOD (TestEmptyCommit) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyForceCommit) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::Committed, e.ForceCommit());
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }

    TEST_METHOD (TestEmptyCancel) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
        Assert::AreEqual(L"", e.RetrieveRaw().c_str());
    }

    // word typing tests

    TEST_METHOD (TestTypingDdoongf) {
        TestValidWord(L"\x111\x1ed3ng", L"ddoongf");
    }

    TEST_METHOD (TestTypingAans) {
        TestValidWord(L"\x1ea5n", L"aans");
    }

    TEST_METHOD (TestTypingDdas) {
        TestValidWord(L"\x111\xe1", L"ddas");
    }

    TEST_METHOD (TestTypingNhuwonxg) {
        TestValidWord(L"nh\x1b0\x1ee1ng", L"nhuwonxg");
    }

    TEST_METHOD (TestTypingNguiw) {
        TestValidWord(L"ng\x1b0i", L"nguiw");
    }

    TEST_METHOD (TestTypingThoio) {
        TestValidWord(L"th\xf4i", L"thoio");
    }

    TEST_METHOD (TestTypingNuaw) {
        TestValidWord(
            L"n\x1b0"
            "a",
            L"nuaw");
    }

    TEST_METHOD (TestTypingQuawms) {
        TestValidWord(L"qu\x1eafm", L"quawms");
    }

    TEST_METHOD (TestTypingQuets) {
        TestValidWord(L"qu\xe9t", L"quets");
    }

    TEST_METHOD (TestTypingQuauj) {
        TestValidWord(L"qu\x1ea1u", L"quauj");
    }

    TEST_METHOD (TestTypingQuoj) {
        TestValidWord(L"qu\x1ecd", L"quoj");
    }

    TEST_METHOD (TestTypingQuitj) {
        TestValidWord(L"qu\x1ecbt", L"quitj");
    }

    TEST_METHOD (TestTypingQueof) {
        TestValidWord(L"qu\xe8o", L"queof");
    }

    TEST_METHOD (TestTypingQuowns) {
        TestValidWord(L"qu\x1edbn", L"quowns");
    }

    TEST_METHOD (TestTypingQuwowns) {
        TestValidWord(L"q\x1b0\x1edbn", L"quwowns");
    }

    TEST_METHOD (TestTypingDduwowcj) {
        TestValidWord(L"\x111\x1b0\x1ee3\x63", L"dduwowcj");
    }

    TEST_METHOD (TestTypingNguwoif) {
        TestValidWord(L"ng\x1b0\x1eddi", L"nguwoif");
    }

    TEST_METHOD (TestTypingThuees) {
        TestValidWord(L"thu\x1ebf", L"thuees");
    }

    // test variations of 'gi'

    TEST_METHOD (TestTypingGif) {
        TestValidWord(L"g\xec", L"gif");
    }

    TEST_METHOD (TestTypingGinf) {
        TestValidWord(L"g\xecn", L"ginf");
    }

    TEST_METHOD (TestTypingGiuowngf) {
        TestValidWord(L"gi\x1b0\x1eddng", L"giuowngf");
    }

    TEST_METHOD (TestTypingGiowf) {
        TestValidWord(L"gi\x1edd", L"giowf");
    }

    TEST_METHOD (TestTypingGiuwax) {
        TestValidWord(
            L"gi\x1eef"
            "a",
            L"giuwax");
    }

    TEST_METHOD (TestTypingGiux) {
        TestValidWord(L"gi\x169", L"giux");
    }

    TEST_METHOD (TestTypingGiuoocj) {
        TestValidWord(
            L"giu\x1ed9"
            "c",
            L"giuoocj");
    }

    // test 'aua' and similar transitions

    TEST_METHOD (TestTypingLauar) {
        TestValidWord(L"l\x1ea9u", L"lauar");
    }

    TEST_METHOD (TestTypingNguayar) {
        TestValidWord(L"ngu\x1ea9y", L"nguayar");
    }

    TEST_METHOD (TestTypingLuuw) {
        TestValidWord(L"l\x1b0u", L"luuw");
    }

    TEST_METHOD (TestTypingHuouw) {
        TestValidWord(L"h\x1b0\x1a1u", L"huouw");
    }

    // irregular

    TEST_METHOD (TestTypingQuoiws) {
        TestValidWord(L"qu\x1edbi", L"quoiws");
    }

    TEST_METHOD (TestTypingDawks) {
        TestValidWord(L"\x111\x1eafk", L"ddawks");
    }

    // peek tests

    TEST_METHOD (TestPeekDd) {
        TestPeekWord(L"\x111", L"dd");
    }

    TEST_METHOD (TestPeekDdd) {
        TestPeekWord(L"dd", L"ddd");
    }

    TEST_METHOD (TestPeekAd) {
        TestPeekWord(L"ad", L"ad");
    }

    TEST_METHOD (TestPeekQuaw) {
        TestPeekWord(L"qu\x103", L"quaw");
    }

    // used to cause a crash
    TEST_METHOD (TestPeekZ) {
        TestPeekWord(L"z", L"z");
    }

    TEST_METHOD (TestPeekCarc) {
        TestPeekWord(L"carc", L"carc");
    }

    TEST_METHOD (TestPeekDdark) {
        TestPeekWord(L"\x111\x1ea3k", L"ddark");
    }

    // test peek key ordering

    TEST_METHOD (TestPeekCace) {
        TestPeekWord(L"cace", L"cace");
    }

    // peek shouldn't crash if tone position is not found
    TEST_METHOD (TestPeekNhaeng) {
        TestPeekWord(L"nhaeng", L"nhaeng");
    }

    // peek key ordering with tones
    TEST_METHOD (TestPeekCafe) {
        TestPeekWord(L"cafe", L"cafe");
    }

    TEST_METHOD (TestPeekDand) {
        auto config1 = config;
        config1.accept_separate_dd = true;
        TelexEngine e(config1);
        FeedWord(e, L"dand");
        Assert::AreEqual(
            L"\x111"
            "an",
            e.Peek().c_str());
    }

    // double key tests

    TEST_METHOD (TestDoubleKeyXuaaan) {
        TestInvalidWord(L"xuaan", L"xuaaan");
    }

    TEST_METHOD (TestDoubleKeyIis) {
        TestInvalidWord(L"iis", L"iis");
    }

    TEST_METHOD (TestDoubleKeyThooongf) {
        TestValidWord(L"tho\xf2ng", L"thooongf");
    }

    TEST_METHOD (TestDoubleKeyThuongz) {
        TestInvalidWord(L"thuongz", L"thuongz");
    }

    TEST_METHOD (TestDoubleKeySystem) {
        TestInvalidWord(L"system", L"system");
    }

    TEST_METHOD (TestNhuwox) {
        TestInvalidWord(L"nhuwox", L"nhuwox");
    }

    TEST_METHOD (TestToool) {
        TestInvalidWord(L"tool", L"toool");
    }

    TEST_METHOD (TestCuwowwcj) {
        TestInvalidWord(L"cuwowcj", L"cuwowwcj");
    }

    TEST_METHOD (TestTypingQuwowwns) {
        TestInvalidWord(L"quwowns", L"quwowwns");
    }

    TEST_METHOD (TestTypingCaasy) {
        TestValidWord(L"c\x1ea5y", L"caasy");
    }

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
        TestInvalidWord(L"OS", L"OSS");
    }

    TEST_METHOD (TestCapsAAA) {
        TestInvalidWord(L"AA", L"AAA");
    }

    TEST_METHOD (TestCapsOWW) {
        TestInvalidWord(L"OW", L"OWW");
    }

    TEST_METHOD (TestCapsGIFF) {
        TestInvalidWord(L"GIF", L"GIFF");
    }

    TEST_METHOD (TestCapsGISS) {
        TestInvalidWord(L"GIS", L"GISS");
    }

    // backspace tests

    TEST_METHOD (TestBackspaceDdoongf) {
        TelexEngine e(config);
        FeedWord(e, L"ddoongf");
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"\x111\x1ed3", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"\x111", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceLeeen) {
        TelexEngine e(config);
        FeedWord(e, L"leeen");
        Assert::AreEqual(L"leen", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
        Assert::AreEqual(L"lee", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceHuowng) {
        TelexEngine e(config);
        FeedWord(e, L"huowng");
        Assert::AreEqual(L"h\x1b0\x1a1ng", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"h\x1b0\x1a1n", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"hu\x1a1", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"hu", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceAoas) {
        TelexEngine e(config);
        FeedWord(e, L"aoas");
        Assert::AreEqual(L"aoas", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"aoa", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceHeei) {
        TelexEngine e(config);
        FeedWord(e, L"heei");
        Assert::AreEqual(L"h\xeai", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"h\xea", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceOwa) {
        TelexEngine e(config);
        FeedWord(e, L"owa");
        Assert::AreEqual(
            L"\x1a1"
            "a",
            e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"\x1a1", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceRuowi) {
        TelexEngine e(config);
        FeedWord(e, L"ruowi");
        Assert::AreEqual(L"r\x1b0\x1a1i", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"ru\x1a1", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceQuee) {
        TelexEngine e(config);
        FeedWord(e, L"quee");
        Assert::AreEqual(L"qu\xea", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"qu", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceQuys) {
        TelexEngine e(config);
        FeedWord(e, L"quys");
        Assert::AreEqual(L"qu\xfd", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"qu", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceHieef) {
        TelexEngine e(config);
        FeedWord(e, L"hieef");
        Assert::AreEqual(L"hi\x1ec1", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"hi", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceGifg) {
        TelexEngine e(config);
        FeedWord(e, L"gifg");
        Assert::AreEqual(L"g\xecg", e.Peek().c_str());
        e.Backspace();
        Assert::AreEqual(L"g\xec", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceXooong) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"xooong"));
        Assert::AreEqual(L"xoong", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"xoon", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"xoo", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceThooongf) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"thooongf"));
        Assert::AreEqual(L"tho\xf2ng", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"tho\xf2n", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"tho\xf2", e.Peek().c_str());
    }

    // test backconversions

    TEST_METHOD (TestBackconversionDdoongf) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"\x111\x1ed3ng"));
        Assert::AreEqual(L"\x111\x1ed3ng", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionSystem) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Invalid, e.Backconvert(L"system"));
        Assert::AreEqual(L"system", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionThees) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"TH\x1ebe"));
        Assert::AreEqual(L"TH\x1ebe", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionVirus) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Invalid, e.Backconvert(L"virus"));
        Assert::AreEqual(L"virus", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionDdoonfCtrlW) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111\x1ed3nw"));
        Assert::AreEqual(L"\x111\x1ed3nw", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
        e.Cancel();
        Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
        Assert::AreEqual(L"\x111\x1ed3n", e.Retrieve().c_str());
    }

    TEST_METHOD (TestBackconversionDdCtrlW) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111w"));
        e.Backspace();
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
    }

    TEST_METHOD (TestBackconversionXooong) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"xoong"));
        Assert::AreEqual(L"xoong", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"xoon", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionXooongUpper) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"XOONG"));
        Assert::AreEqual(L"XOONG", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionThooongf) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"tho\xf2ng"));
        Assert::AreEqual(L"tho\xf2ng", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"tho\xf2n", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionThooongfUpper) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"THO\xd2NG"));
        Assert::AreEqual(L"THO\xd2NG", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionXoooong) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"x\xf4\xf4ng"));
        Assert::AreEqual(L"x\xf4\xf4ng", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionXo_oong) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"xo\xf4ng"));
        Assert::AreEqual(L"xo\xf4ng", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backspace());
        Assert::AreEqual(L"xo\xf4n", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backspace());
        Assert::AreEqual(L"xo\xf4", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"xo", e.Peek().c_str());
    }

    TEST_METHOD (TestBackconversionCaays) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"c\x1ea5y"));
        e.Commit();
        Assert::AreEqual(L"c\x1ea5y", e.Retrieve().c_str());
    }

    TEST_METHOD (TestBackconversionQuaays) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"qu\x1ea5y"));
        e.Commit();
        Assert::AreEqual(L"qu\x1ea5y", e.Retrieve().c_str());
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
        TelexEngine e(config1);
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
    }

    // test dd accept

    TEST_METHOD (TestValidDodongf) {
        TestValidWord(L"\x111\x1ed3ng", L"dodongf");
    }

    TEST_METHOD (TestInvalidDodongf) {
        auto config1 = config;
        config1.accept_separate_dd = false;
        TelexEngine e(config1);
        VietType::UnitTests::TestInvalidWord(e, L"dodongf", L"dodongf");
    }

    TEST_METHOD (TestConfigOaUy) {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        TelexEngine e(config1);
        VietType::UnitTests::TestValidWord(e, L"to\xe0n", L"toanf");
    }

    // test multilang optimizations
    TEST_METHOD (TestMultilangVirus) {
        auto config1 = config;
        config1.optimize_multilang = TelexConfig::OptimizeMultilang::On;
        TelexEngine e(config1);
        VietType::UnitTests::TestInvalidWord(e, L"virus", L"virus");
    }

    TEST_METHOD (TestMultilangDefe) {
        auto config1 = config;
        config1.optimize_multilang = TelexConfig::OptimizeMultilang::Aggressive;
        TelexEngine e(config1);
        AssertTelexStatesEqual(TelexStates::Invalid, FeedWord(e, L"defe"));
    }

    // test doublekey backspace

    TEST_METHOD (TestBackspaceMooo) {
        TelexEngine e(config);
        FeedWord(e, L"mooo");
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"mo", e.Peek().c_str());
    }

    TEST_METHOD (TestBackspaceMooof) {
        TelexEngine e(config);
        FeedWord(e, L"mooof");
        AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
        Assert::AreEqual(L"mo", e.Peek().c_str());
    }
};

} // namespace UnitTests
} // namespace VietType
