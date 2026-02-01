// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestTelex", "[telex]") {
    TelexConfig config{
        .typing_style = TypingStyles::Telex,
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

    SECTION("reset") {
        engine->Reset();
        CHECK(L"" == engine->Retrieve());
        CHECK(L"" == engine->RetrieveRaw());
        CHECK(L"" == engine->Peek());
        CHECK(std::size_t{0} == engine->Count());
    }

    SECTION("push char") {
        SECTION("TestTelexEmptyPushCharC1_1") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'b'));
            CHECK(L"b" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyPushCharC1_2") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'c'));
            CHECK(L"c" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyPushCharC1_3") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'd'));
            CHECK(L"d" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyPushCharC1_4") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'r'));
            CHECK(L"r" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyPushCharC1_5") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'x'));
            CHECK(L"x" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyPushCharV") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'a'));
            CHECK(L"a" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyCommittedPushChar") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
            AssertTelexStatesEqual(TelexStates::Committed, engine->PushChar('a'));
            CHECK(L"" == engine->Retrieve());
            CHECK(L"" == engine->RetrieveRaw());
            CHECK(L"" == engine->Peek());
            CHECK(std::size_t{0} == engine->Count());
        }
    }

    SECTION("uppercase") {
        SECTION("TestTelexEmptyPushCharUpC1") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'B'));
            CHECK(L"B" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyPushCharUpV") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->PushChar(L'A'));
            CHECK(L"A" == engine->Retrieve());
        }

        SECTION("TestTelexTypingUpAai") {
            TestValidWord(L"\xc2n", L"AAn");
            TestValidWord(L"\xc2n", L"Aan");
        }

        SECTION("TestTelexTypingUpDdi") {
            TestValidWord(L"\x110i", L"DDi");
            TestValidWord(L"\x110i", L"Ddi");
        }

        SECTION("TestTelexTypingUpAasn") {
            TestValidWord(L"\x1ea4n", L"AASn");
            TestValidWord(L"\x1ea4n", L"Aasn");
            TestValidWord(L"\x1ea4n", L"AAsn");
        }
    }

    SECTION("backspace") {
        SECTION("TestTelexEmptyBackspace") {
            engine->Reset();
            CHECK(L"" == engine->Retrieve());
            CHECK(L"" == engine->RetrieveRaw());
        }

        SECTION("TestTelexBackspaceAfzea") {
            engine->Reset();
            if (FeedWord(*engine, L"afzea") == TelexStates::Valid) {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"ae" == engine->Retrieve());
            }
        }
    }

    SECTION("commit") {
        SECTION("TestTelexEmptyCommit") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
            CHECK(L"" == engine->Retrieve());
        }

        SECTION("TestTelexEmptyCancel") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::CommittedInvalid, engine->Cancel());
            CHECK(L"" == engine->RetrieveRaw());
        }

        SECTION("TestTelexEmptyBackconversion") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(std::wstring()));
            CHECK(L"" == engine->RetrieveRaw());
        }
    }

    SECTION("word typing tests") {
        TestValidWord(L"\x111\x1ed3ng", L"ddoongf");
        TestValidWord(L"\x1ea5n", L"aans");
        TestValidWord(L"\x111\xe1", L"ddas");
        TestValidWord(L"nh\x1b0\x1ee1ng", L"nhuwonxg");
        TestValidWord(L"ng\x1b0i", L"nguiw");
        TestValidWord(L"th\xf4i", L"thoio");
        TestValidWord(L"n\x1b0\x61", L"nuaw");
        TestValidWord(L"qu\x1eafm", L"quawms");
        TestValidWord(L"qu\xe9t", L"quets");
        TestValidWord(L"qu\x1ea1u", L"quauj");
        TestValidWord(L"qu\x1ecd", L"quoj");
        TestValidWord(L"qu\x1ecbt", L"quitj");
        TestValidWord(L"qu\xe8o", L"queof");
        TestValidWord(L"qu\x1edbn", L"quowns");
        TestValidWord(L"q\x1b0\x1edbn", L"quwowns");
        TestValidWord(L"qu\x1ee1n", L"quonwx");
        TestValidWord(L"\x111\x1b0\x1ee3\x63", L"dduwowcj");
        TestValidWord(L"ng\x1b0\x1eddi", L"nguwoif");
        TestValidWord(L"thu\x1ebf", L"thuees");
        TestValidWord(L"khu\x1eafng", L"khuawngs");
        TestValidWord(L"khu\x1eafng", L"khuawsng");
        TestValidWord(L"ra", L"ra");
    }

    SECTION("test variations of 'gi'") {
        TestValidWord(L"g\xec", L"gif");
        TestValidWord(L"g\xecn", L"ginf");
        TestValidWord(L"gi\x1b0\x1eddng", L"giuowngf");
        TestValidWord(L"gi\x1edd", L"giowf");
        TestValidWord(L"gi\x1eef\x61", L"giuwax");
        TestValidWord(L"gi\x169", L"giux");
        TestValidWord(L"giu\x1ed9\x63", L"giuoocj");
        TestValidWord(L"gi\xe8m", L"giemf");
        TestValidWord(L"gi\xea", L"giee");
    }

    SECTION("test 'aua' and similar transitions") {
        TestValidWord(L"l\x1ea9u", L"lauar");
        TestValidWord(L"ngu\x1ea9y", L"nguayar");
        TestValidWord(L"l\x1b0u", L"luuw");
        TestValidWord(L"h\x1b0\x1a1u", L"huouw");
    }

    SECTION("irregular") {
        TestValidWord(L"qu\x1edbi", L"quoiws");
        TestValidWord(L"\x111\x1eafk", L"ddawks");
    }

    SECTION("peek tests") {
        TestPeekWord(L"\x111", L"dd");
        TestPeekWord(L"dd", L"ddd");
        TestPeekWord(L"ad", L"ad");
        TestPeekWord(L"qu\x103", L"quaw");
        TestPeekWord(L"z", L"z");
        TestPeekWord(L"carc", L"carc");
        TestPeekWord(L"\x111\x1ea3k", L"ddark");
        TestPeekWord(L"cace", L"cace");
        TestPeekWord(L"nhaeng", L"nhaeng");
        TestPeekWord(L"cafe", L"cafe");
    }

    SECTION("double key tests") {
        TestInvalidWord(L"xuaan", L"xuaaan");
        TestInvalidWord(L"iis", L"iis");
        TestValidWord(L"tho\xf2ng", L"thooongf");
        TestInvalidWord(L"thuongz", L"thuongz");
        TestInvalidWord(L"system", L"system");
        TestInvalidWord(L"nhuwox", L"nhuwox");
        TestInvalidWord(L"tool", L"toool");
        TestInvalidWord(L"cuwowcj", L"cuwowwcj");
        TestInvalidWord(L"quwowns", L"quwowwns");
        TestInvalidWord(L"khongoo", L"khongoo");
        TestValidWord(L"kh\xf4ng", L"khongo");
        TestValidWord(L"c\x1ea5y", L"caasy");
    }

    SECTION("caps") {
        TestPeekWord(L"D", L"D");
        TestValidWord(L"X\x1b0\x1edcng", L"XuOwngf");
        TestInvalidWord(L"XuOnWfWx", L"XuOnWfWx");
        TestValidWord(L"NG\x1af\x1edCI", L"NGUOIWF");
    }

    SECTION("caps repeat") {
        TestInvalidWord(L"AA", L"AAA");
        TestInvalidWord(L"OW", L"OWW");
        if (config.optimize_multilang <= 1) {
            TestPeekWord(L"OS", L"OSS");
            TestPeekWord(L"GIF", L"GIFF");
            TestPeekWord(L"GIS", L"GISS");
        }
    }

    SECTION("backspace tests") {
        SECTION("TestTelexBackspaceDdoongf") {
            FeedWord(*engine, L"ddoongf");
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"\x111\x1ed3n" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"\x111\x1ed3" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"\x111" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"" == engine->Peek());
        }

        SECTION("TestTelexBackspaceLeeen") {
            FeedWord(*engine, L"leeen");
            CHECK(L"leen" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Invalid, engine->Backspace());
            CHECK(L"lee" == engine->Peek());
        }

        SECTION("TestTelexBackspaceHuowng") {
            FeedWord(*engine, L"huowng");
            CHECK(L"h\x1b0\x1a1ng" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"h\x1b0\x1a1n" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"hu\x1a1" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"hu" == engine->Peek());
        }

        SECTION("TestTelexBackspaceAoas") {
            FeedWord(*engine, L"aoas");
            CHECK(L"aoas" == engine->Peek());
            engine->Backspace();
            CHECK(L"aoa" == engine->Peek());
        }

        SECTION("TestTelexBackspaceHeei") {
            FeedWord(*engine, L"heei");
            CHECK(L"h\xeai" == engine->Peek());
            engine->Backspace();
            CHECK(L"h\xea" == engine->Peek());
        }

        SECTION("TestTelexBackspaceOwa") {
            FeedWord(*engine, L"owa");
            CHECK(L"\x1a1\x61" == engine->Peek());
            engine->Backspace();
            CHECK(L"\x1a1" == engine->Peek());
        }

        SECTION("TestTelexBackspaceRuowi") {
            FeedWord(*engine, L"ruowi");
            CHECK(L"r\x1b0\x1a1i" == engine->Peek());
            engine->Backspace();
            CHECK(L"ru\x1a1" == engine->Peek());
        }

        SECTION("TestTelexBackspaceQuee") {
            FeedWord(*engine, L"quee");
            CHECK(L"qu\xea" == engine->Peek());
            engine->Backspace();
            CHECK(L"qu" == engine->Peek());
        }

        SECTION("TestTelexBackspaceQuys") {
            FeedWord(*engine, L"quys");
            CHECK(L"qu\xfd" == engine->Peek());
            engine->Backspace();
            CHECK(L"qu" == engine->Peek());
        }

        SECTION("TestTelexBackspaceHieef") {
            FeedWord(*engine, L"hieef");
            CHECK(L"hi\x1ec1" == engine->Peek());
            engine->Backspace();
            CHECK(L"hi" == engine->Peek());
        }

        SECTION("TestTelexBackspaceGifg") {
            FeedWord(*engine, L"gifg");
            CHECK(L"g\xecg" == engine->Peek());
            engine->Backspace();
            CHECK(L"g\xec" == engine->Peek());
            engine->Backspace();
            CHECK(L"g" == engine->Peek());
            engine->PushChar(L'i');
            CHECK(L"gi" == engine->Peek());
        }

        SECTION("TestTelexBackspaceDdayas") {
            FeedWord(*engine, L"ddayas");
            CHECK(L"\x111\x1ea5y" == engine->Peek());
            engine->Backspace();
            CHECK(L"\x111\x1ea5" == engine->Peek());
            engine->Backspace();
            CHECK(L"\x111" == engine->Peek());
        }

        SECTION("TestTelexBackspaceDdaysa") {
            if (config.optimize_multilang <= 2) {
                FeedWord(*engine, L"ddaysa");
                CHECK(L"\x111\x1ea5y" == engine->Peek());
                engine->Backspace();
                CHECK(L"\x111\x1ea5" == engine->Peek());
                engine->Backspace();
                CHECK(L"\x111" == engine->Peek());
            }
        }

        SECTION("TestTelexBackspaceHofazn") {
            if (config.optimize_multilang == 0) {
                FeedWord(*engine, L"hofazn");
                CHECK(L"hoan" == engine->Peek());
                engine->Backspace();
                CHECK(L"hoa" == engine->Peek());
            }
        }

        SECTION("TestTelexBackspaceXooong") {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"xooong"));
            CHECK(L"xoong" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"xoon" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"xoo" == engine->Peek());
        }

        SECTION("TestTelexBackspaceThooongf") {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"thooongf"));
            CHECK(L"tho\xf2ng" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"tho\xf2n" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"tho\xf2" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"tho" == engine->Peek());
        }

        SECTION("TestTelexBackspaceAssi") {
            if (config.optimize_multilang <= 1) {
                auto config1 = config;
                config1.backspaced_word_stays_invalid = false;
                auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));
                FeedWord(*e, L"asssi");
                CHECK(L"assi" == e->Peek());
                e->Backspace();
                CHECK(L"ass" == e->Peek());
            }
        }
    }

    SECTION("test tone and w movements") {
        SECTION("TestTelexBackspaceCuwsoc") {
            if (config.optimize_multilang <= 2) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cuwsoc"));
                CHECK(L"c\x1b0\x1edb\x63" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"c\x1b0\x1edb" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"c\x1b0" == engine->Peek());
            }
        }
        SECTION("TestTelexBackspaceCuwocs") {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cuwocs"));
            CHECK(L"c\x1b0\x1edb\x63" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"c\x1b0\x1edb" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"c\x1b0" == engine->Peek());
        }
        SECTION("TestTelexBackspaceCuocws") {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cuocws"));
            CHECK(L"c\x1b0\x1edb\x63" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"cu\x1edb" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"cu" == engine->Peek());
        }
        SECTION("TestTelexBackspaceCows") {
            AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cows"));
            CHECK(L"c\x1edb" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"c" == engine->Peek());
            engine->PushChar(L'u');
            CHECK(L"cu" == engine->Peek());
        }
        SECTION("TestTelexBackspaceEefsch") {
            if (FeedWord(*engine, L"eefsch") == TelexStates::Valid) {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"\x1ebf\x63" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"\x1ebf" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"" == engine->Peek());
            }
        }
    }

    SECTION("test backconversions") {
        SECTION("TestTelexBackconversionDdoongf") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"\x111\x1ed3ng"));
            CHECK(L"\x111\x1ed3ng" == engine->Peek());
        }
        SECTION("TestTelexBackconversionSystem") {
            AssertTelexStatesEqual(TelexStates::Invalid, engine->Backconvert(L"system"));
            CHECK(L"system" == engine->Peek());
        }
        SECTION("TestTelexBackconversionThees") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"TH\x1ebe"));
            CHECK(L"TH\x1ebe" == engine->Peek());
        }
        SECTION("TestTelexBackconversionVirus") {
            AssertTelexStatesEqual(TelexStates::Invalid, engine->Backconvert(L"virus"));
            CHECK(L"virus" == engine->Peek());
        }
        SECTION("TestTelexBackconversionDdoonfCtrlW") {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"\x111\x1ed3nw"));
            CHECK(L"\x111\x1ed3nw" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"\x111\x1ed3n" == engine->Peek());
            engine->Cancel();
            CHECK(L"\x111\x1ed3n" == engine->Peek());
            CHECK(L"\x111\x1ed3n" == engine->Retrieve());
        }
        SECTION("TestTelexBackconversionDdCtrlW") {
            if (!config.autocorrect) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"\x111w"));
                engine->Backspace();
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                AssertTelexStatesEqual(TelexStates::CommittedInvalid, engine->Cancel());
            }
        }
        SECTION("TestTelexBackconversionXooong") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"xoong"));
            CHECK(L"xoong" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"xoon" == engine->Peek());
        }
        SECTION("TestTelexBackconversionXooongUpper") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"XOONG"));
            CHECK(L"XOONG" == engine->Peek());
        }
        SECTION("TestTelexBackconversionThooongf") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"tho\xf2ng"));
            CHECK(L"tho\xf2ng" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"tho\xf2n" == engine->Peek());
        }
        SECTION("TestTelexBackconversionThooongfUpper") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"THO\xd2NG"));
            CHECK(L"THO\xd2NG" == engine->Peek());
        }
        SECTION("TestTelexBackconversionXoooong") {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"x\xf4\xf4ng"));
            CHECK(L"x\xf4\xf4ng" == engine->Peek());
        }
        SECTION("TestTelexBackconversionXo_oong") {
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"xo\xf4ng"));
            CHECK(L"xo\xf4ng" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backspace());
            CHECK(L"xo\xf4n" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backspace());
            CHECK(L"xo\xf4" == engine->Peek());
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"xo" == engine->Peek());
        }
        SECTION("TestTelexBackconversionCaays") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"c\x1ea5y"));
            engine->Commit();
            CHECK(L"c\x1ea5y" == engine->Retrieve());
        }
        SECTION("TestTelexBackconversionQuaays") {
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"qu\x1ea5y"));
            engine->Commit();
            CHECK(L"qu\x1ea5y" == engine->Retrieve());
        }
    }

    SECTION("test oa/oe/uy") {
        TestValidWord(L"to\xe0n", L"toanf");
        if (config.oa_uy_tone1) {
            TestValidWord(L"ho\xe0", L"hoaf");
            TestValidWord(L"ho\xe8", L"hoef");
            TestValidWord(L"lu\x1ef5", L"luyj");
        } else {
            TestValidWord(L"h\xf2\x61", L"hoaf");
            TestValidWord(L"h\xf2\x65", L"hoef");
            TestValidWord(L"l\x1ee5y", L"luyj");
        }
    }

    SECTION("test doublekey backspace") {
        SECTION("TestTelexBackspaceMooo") {
            FeedWord(*engine, L"mooo");
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"mo" == engine->Peek());
        }
        SECTION("TestTelexBackspaceMooof") {
            FeedWord(*engine, L"mooof");
            AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
            CHECK(L"mo" == engine->Peek());
        }
    }

    SECTION("autocorrect") {
        if (config.autocorrect) {
            SECTION("TestTelexAutocorrectHwuogn") {
                FeedWord(*engine, L"hwuogn");
                AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
                CHECK(L"h\x1b0\x1a1ng" == engine->Retrieve());
            }
            SECTION("TestTelexAutocorrectViets") {
                FeedWord(*engine, L"viets");
                AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
                CHECK(L"vi\x1ebft" == engine->Retrieve());
            }
            SECTION("TestTelexTypingThuowgnf") {
                FeedWord(*engine, L"thuowgnf");
                AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
                CHECK(L"th\x1b0\x1eddng" == engine->Retrieve());
            }
        }
    }

    SECTION("test dd accept") {
        if (config.accept_separate_dd) {
            SECTION("TestTelexPeekDand") {
                FeedWord(*engine, L"dand");
                CHECK(L"\x111\x61n" == engine->Peek());
            }
            SECTION("TestTelexValidDodongf") {
                TestValidWord(L"\x111\x1ed3ng", L"dodongf");
            }
        } else {
            TestInvalidWord(L"dodongf", L"dodongf");
        }
    }

    SECTION("test abbreviations") {
        if (config.allow_abbreviations) {
            TestValidWord(L"\x111\x63", L"ddc");
            TestInvalidWord(L"ddca", L"ddca");
            TestValidWord(L"q\x111", L"qdd");
            TestInvalidWord(L"qdda", L"qdda");
        }
    }

    SECTION("test no abbreviations") {
        if (!config.allow_abbreviations) {
            TestInvalidWord(L"ddc", L"ddc");
            TestInvalidWord(L"qdd", L"qdd");
            TestInvalidWord(L"qdda", L"qdda");
        }
    }

    SECTION("test multilang optimizations") {
        SECTION("TestTelexMultilangVirus") {
            if (config.optimize_multilang == 1) {
                TestInvalidWord(L"virus", L"virus");
            }
        }
        SECTION("TestTelexMultilangDense") {
            if (config.optimize_multilang == 2) {
                TestInvalidWord(L"dense", L"dense");
            }
        }
        SECTION("TestTelexMultilangDefe") {
            if (config.optimize_multilang == 3) {
                AssertTelexStatesEqual(TelexStates::Invalid, FeedWord(*engine, L"defe"));
            }
        }
        SECTION("TestTelexMultilangVirusUpper") {
            if (config.optimize_multilang == 1) {
                TestInvalidWord(L"VIRUS", L"VIRUS");
            }
        }
        SECTION("TestTelexMultilangDenseUpper") {
            if (config.optimize_multilang == 2) {
                TestInvalidWord(L"DENSE", L"DENSE");
            }
        }
    }
}

} // namespace UnitTests
} // namespace VietType
