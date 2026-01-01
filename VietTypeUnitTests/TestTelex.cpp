// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestTelex", "[telex]") {
    auto level = GENERATE(0, 1, 2, 3);
    auto autocorrect = GENERATE(true, false);

    TelexConfig config{};
    config.optimize_multilang = level;
    config.autocorrect = autocorrect;

    SECTION("engine") {
        auto engine = std::unique_ptr<ITelexEngine>(TelexNew(config));

        auto TestValidWord = [&](const wchar_t* expected, const wchar_t* input) {
            UnitTests::TestValidWord(*engine, expected, input);
        };

        auto TestInvalidWord = [&](const wchar_t* expected, const wchar_t* input) {
            UnitTests::TestInvalidWord(*engine, expected, input);
        };

        auto TestPeekWord =
            [&](const wchar_t* expected, const wchar_t* input, TelexStates state = TelexStates::TxError) {
                UnitTests::TestPeekWord(*engine, expected, input);
                if (state != TelexStates::TxError) {
                    AssertTelexStatesEqual(state, engine->GetState());
                }
            };

        SECTION("reset") {
            SECTION("TestTelexEmpty") {
                engine->Reset();
                CHECK(L"" == engine->Retrieve());
                CHECK(L"" == engine->RetrieveRaw());
                CHECK(L"" == engine->Peek());
                CHECK(std::size_t{0} == engine->Count());
            }
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
                SECTION("1") {
                    TestValidWord(L"\xc2n", L"AAn");
                }
                SECTION("2") {
                    TestValidWord(L"\xc2n", L"Aan");
                }
            }

            SECTION("TestTelexTypingUpDdi") {
                SECTION("1") {
                    TestValidWord(L"\x110i", L"DDi");
                }
                SECTION("2") {
                    TestValidWord(L"\x110i", L"Ddi");
                }
            }

            SECTION("TestTelexTypingUpAasn") {
                SECTION("1") {
                    TestValidWord(L"\x1ea4n", L"AASn");
                }
                SECTION("2") {
                    TestValidWord(L"\x1ea4n", L"Aasn");
                }
                SECTION("3") {
                    TestValidWord(L"\x1ea4n", L"AAsn");
                }
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
            SECTION("TestTelexTypingDdoongf") {
                TestValidWord(L"\x111\x1ed3ng", L"ddoongf");
            }
            SECTION("TestTelexTypingAans") {
                TestValidWord(L"\x1ea5n", L"aans");
            }
            SECTION("TestTelexTypingDdas") {
                TestValidWord(L"\x111\xe1", L"ddas");
            }
            SECTION("TestTelexTypingNhuwonxg") {
                TestValidWord(L"nh\x1b0\x1ee1ng", L"nhuwonxg");
            }
            SECTION("TestTelexTypingNguiw") {
                TestValidWord(L"ng\x1b0i", L"nguiw");
            }
            SECTION("TestTelexTypingThoio") {
                TestValidWord(L"th\xf4i", L"thoio");
            }
            SECTION("TestTelexTypingNuaw") {
                TestValidWord(L"n\x1b0\x61", L"nuaw");
            }
            SECTION("TestTelexTypingQuawms") {
                TestValidWord(L"qu\x1eafm", L"quawms");
            }
            SECTION("TestTelexTypingQuets") {
                TestValidWord(L"qu\xe9t", L"quets");
            }
            SECTION("TestTelexTypingQuauj") {
                TestValidWord(L"qu\x1ea1u", L"quauj");
            }
            SECTION("TestTelexTypingQuoj") {
                TestValidWord(L"qu\x1ecd", L"quoj");
            }
            SECTION("TestTelexTypingQuitj") {
                TestValidWord(L"qu\x1ecbt", L"quitj");
            }
            SECTION("TestTelexTypingQueof") {
                TestValidWord(L"qu\xe8o", L"queof");
            }
            SECTION("TestTelexTypingQuowns") {
                TestValidWord(L"qu\x1edbn", L"quowns");
            }
            SECTION("TestTelexTypingQuwowns") {
                TestValidWord(L"q\x1b0\x1edbn", L"quwowns");
            }
            SECTION("TestTelexTypingQuonwx") {
                TestValidWord(L"qu\x1ee1n", L"quonwx");
            }
            SECTION("TestTelexTypingDduwowcj") {
                TestValidWord(L"\x111\x1b0\x1ee3\x63", L"dduwowcj");
            }
            SECTION("TestTelexTypingNguwoif") {
                TestValidWord(L"ng\x1b0\x1eddi", L"nguwoif");
            }
            SECTION("TestTelexTypingThuees") {
                TestValidWord(L"thu\x1ebf", L"thuees");
            }
            SECTION("TestTelexTypingKhuawngs") {
                TestValidWord(L"khu\x1eafng", L"khuawngs");
            }
            SECTION("TestTelexTypingKhuawsng") {
                TestValidWord(L"khu\x1eafng", L"khuawsng");
            }
            SECTION("TestTelexTypingRa") {
                TestValidWord(L"ra", L"ra");
            }
        }

        SECTION("test variations of 'gi'") {
            SECTION("TestTelexTypingGif") {
                TestValidWord(L"g\xec", L"gif");
            }
            SECTION("TestTelexTypingGinf") {
                TestValidWord(L"g\xecn", L"ginf");
            }
            SECTION("TestTelexTypingGiuowngf") {
                TestValidWord(L"gi\x1b0\x1eddng", L"giuowngf");
            }
            SECTION("TestTelexTypingGiowf") {
                TestValidWord(L"gi\x1edd", L"giowf");
            }
            SECTION("TestTelexTypingGiuwax") {
                TestValidWord(L"gi\x1eef\x61", L"giuwax");
            }
            SECTION("TestTelexTypingGiux") {
                TestValidWord(L"gi\x169", L"giux");
            }
            SECTION("TestTelexTypingGiuoocj") {
                TestValidWord(L"giu\x1ed9\x63", L"giuoocj");
            }
            SECTION("TestTelexTypingGiemf") {
                TestValidWord(L"gi\xe8m", L"giemf");
            }
            SECTION("TestTelexTypingGiee") {
                TestValidWord(L"gi\xea", L"giee");
            }
        }

        SECTION("test 'aua' and similar transitions") {
            SECTION("TestTelexTypingLauar") {
                TestValidWord(L"l\x1ea9u", L"lauar");
            }
            SECTION("TestTelexTypingNguayar") {
                TestValidWord(L"ngu\x1ea9y", L"nguayar");
            }
            SECTION("TestTelexTypingLuuw") {
                TestValidWord(L"l\x1b0u", L"luuw");
            }
            SECTION("TestTelexTypingHuouw") {
                TestValidWord(L"h\x1b0\x1a1u", L"huouw");
            }
        }

        SECTION("irregular") {
            SECTION("TestTelexTypingQuoiws") {
                TestValidWord(L"qu\x1edbi", L"quoiws");
            }
            SECTION("TestTelexTypingDawks") {
                TestValidWord(L"\x111\x1eafk", L"ddawks");
            }
        }

        SECTION("peek tests") {
            SECTION("TestTelexPeekDd") {
                TestPeekWord(L"\x111", L"dd");
            }
            SECTION("TestTelexPeekDdd") {
                TestPeekWord(L"dd", L"ddd");
            }
            SECTION("TestTelexPeekAd") {
                TestPeekWord(L"ad", L"ad");
            }
            SECTION("TestTelexPeekQuaw") {
                TestPeekWord(L"qu\x103", L"quaw");
            }
            SECTION("TestTelexPeekZ") {
                TestPeekWord(L"z", L"z");
            }
            SECTION("TestTelexPeekCarc") {
                TestPeekWord(L"carc", L"carc");
            }
            SECTION("TestTelexPeekDdark") {
                TestPeekWord(L"\x111\x1ea3k", L"ddark");
            }
            SECTION("TestTelexPeekCace") {
                TestPeekWord(L"cace", L"cace");
            }
            SECTION("TestTelexPeekNhaeng") {
                TestPeekWord(L"nhaeng", L"nhaeng");
            }
            SECTION("TestTelexPeekCafe") {
                TestPeekWord(L"cafe", L"cafe");
            }
        }

        SECTION("double key tests") {
            SECTION("TestTelexDoubleKeyXuaaan") {
                TestInvalidWord(L"xuaan", L"xuaaan");
            }
            SECTION("TestTelexDoubleKeyIis") {
                TestInvalidWord(L"iis", L"iis");
            }
            SECTION("TestTelexDoubleKeyThooongf") {
                TestValidWord(L"tho\xf2ng", L"thooongf");
            }
            SECTION("TestTelexDoubleKeyThuongz") {
                TestInvalidWord(L"thuongz", L"thuongz");
            }
            SECTION("TestTelexDoubleKeySystem") {
                TestInvalidWord(L"system", L"system");
            }
            SECTION("TestTelexNhuwox") {
                TestInvalidWord(L"nhuwox", L"nhuwox");
            }
            SECTION("TestTelexToool") {
                TestInvalidWord(L"tool", L"toool");
            }
            SECTION("TestTelexCuwowwcj") {
                TestInvalidWord(L"cuwowcj", L"cuwowwcj");
            }
            SECTION("TestTelexTypingQuwowwns") {
                TestInvalidWord(L"quwowns", L"quwowwns");
            }
            SECTION("TestTelexKhongoo") {
                TestInvalidWord(L"khongoo", L"khongoo");
            }
            SECTION("TestTelexKhongo") {
                TestValidWord(L"kh\xf4ng", L"khongo");
            }
            SECTION("TestTelexTypingCaasy") {
                TestValidWord(L"c\x1ea5y", L"caasy");
            }
        }

        SECTION("caps") {
            SECTION("TestTelexCapsPeekD") {
                TestPeekWord(L"D", L"D");
            }
            SECTION("TestTelexCapsXuOwngf") {
                TestValidWord(L"X\x1b0\x1edcng", L"XuOwngf");
            }
            SECTION("TestTelexCapsXuOnWfWx") {
                TestInvalidWord(L"XuOnWfWx", L"XuOnWfWx");
            }
            SECTION("TestTelexAllCapsNguoiwf") {
                TestValidWord(L"NG\x1af\x1edCI", L"NGUOIWF");
            }
        }

        SECTION("caps repeat") {
            SECTION("TestTelexCapsOSS") {
                if (level <= 1) {
                    UnitTests::TestPeekWord(*engine, L"OS", L"OSS");
                }
            }
            SECTION("TestTelexCapsAAA") {
                TestInvalidWord(L"AA", L"AAA");
            }
            SECTION("TestTelexCapsOWW") {
                TestInvalidWord(L"OW", L"OWW");
            }
            SECTION("TestTelexCapsGIFF") {
                if (level <= 1) {
                    UnitTests::TestPeekWord(*engine, L"GIF", L"GIFF");
                }
            }
            SECTION("TestTelexCapsGISS") {
                if (level <= 1) {
                    UnitTests::TestPeekWord(*engine, L"GIS", L"GISS");
                }
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

            SECTION("TestTelexBackspaceHofazn") {
                if (level == 0) {
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
                if (level <= 1) {
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
                if (level <= 2) {
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
                if (!autocorrect) {
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
            SECTION("TestTelexTypingOaUy") {
                TestValidWord(L"ho\xe0", L"hoaf");
                TestValidWord(L"ho\xe8", L"hoef");
                TestValidWord(L"lu\x1ef5", L"luyj");
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
            if (autocorrect) {
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
    }

    SECTION("test dd accept") {
        auto config1 = config;
        config1.accept_separate_dd = true;
        auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));

        SECTION("TestTelexPeekDand") {
            FeedWord(*e, L"dand");
            CHECK(L"\x111\x61n" == e->Peek());
        }
        SECTION("TestTelexValidDodongf") {
            UnitTests::TestValidWord(*e, L"\x111\x1ed3ng", L"dodongf");
        }
    }

    SECTION("test abbreviations") {
        auto config1 = config;
        config1.allow_abbreviations = true;
        auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));

        SECTION("ddc") {
            UnitTests::TestValidWord(*e, L"\x111\x63", L"ddc");
        }
        SECTION("ddca") {
            UnitTests::TestInvalidWord(*e, L"ddca", L"ddca");
        }
        SECTION("qdd") {
            UnitTests::TestValidWord(*e, L"q\x111", L"qdd");
        }
        SECTION("qdda") {
            UnitTests::TestInvalidWord(*e, L"qdda", L"qdda");
        }
    }

    SECTION("test no abbreviations") {
        auto config1 = config;
        config1.allow_abbreviations = false;
        auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));

        SECTION("ddc") {
            UnitTests::TestInvalidWord(*e, L"ddc", L"ddc");
        }
        SECTION("qdd") {
            UnitTests::TestInvalidWord(*e, L"qdd", L"qdd");
        }
        SECTION("qdda") {
            UnitTests::TestInvalidWord(*e, L"qdda", L"qdda");
        }
    }

    SECTION("TestTelexTypingOaUyOff") {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));
        UnitTests::TestValidWord(*e, L"to\xe0n", L"toanf");
        UnitTests::TestValidWord(*e, L"h\xf2\x61", L"hoaf");
        UnitTests::TestValidWord(*e, L"h\xf2\x65", L"hoef");
        UnitTests::TestValidWord(*e, L"l\x1ee5y", L"luyj");
    }

    SECTION("test multilang optimizations") {
        SECTION("TestTelexMultilangVirus") {
            auto config1 = config;
            config1.optimize_multilang = 1;
            std::unique_ptr<ITelexEngine> e(TelexNew(config1));
            UnitTests::TestInvalidWord(*e, L"virus", L"virus");
        }
        SECTION("TestTelexMultilangDense") {
            auto config1 = config;
            config1.optimize_multilang = 2;
            std::unique_ptr<ITelexEngine> e(TelexNew(config1));
            UnitTests::TestInvalidWord(*e, L"dense", L"dense");
        }
        SECTION("TestTelexMultilangDefe") {
            auto config1 = config;
            config1.optimize_multilang = 3;
            std::unique_ptr<ITelexEngine> e(TelexNew(config1));
            AssertTelexStatesEqual(TelexStates::Invalid, FeedWord(*e, L"defe"));
        }
        SECTION("TestTelexMultilangVirusUpper") {
            auto config1 = config;
            config1.optimize_multilang = 1;
            std::unique_ptr<ITelexEngine> e(TelexNew(config1));
            UnitTests::TestInvalidWord(*e, L"VIRUS", L"VIRUS");
        }
        SECTION("TestTelexMultilangDenseUpper") {
            auto config1 = config;
            config1.optimize_multilang = 2;
            std::unique_ptr<ITelexEngine> e(TelexNew(config1));
            UnitTests::TestInvalidWord(*e, L"DENSE", L"DENSE");
        }
    }
}

} // namespace UnitTests
} // namespace VietType
