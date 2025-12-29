// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestTelex", "[telex]") {
    const TelexConfig config{};

    auto TestValidWord = [&](const wchar_t* expected, const wchar_t* input) {
        MultiConfigTester(config).Invoke([=](auto& e) { UnitTests::TestValidWord(e, expected, input); });
    };

    auto TestInvalidWord = [&](const wchar_t* expected, const wchar_t* input) {
        MultiConfigTester(config).Invoke([=](auto& e) { UnitTests::TestInvalidWord(e, expected, input); });
    };

    auto TestPeekWord = [&](const wchar_t* expected, const wchar_t* input, TelexStates state = TelexStates::TxError) {
        MultiConfigTester(config).Invoke([=](auto& e) {
            UnitTests::TestPeekWord(e, expected, input);
            if (state != TelexStates::TxError) {
                AssertTelexStatesEqual(state, e.GetState());
            }
        });
    };

    SECTION("reset") {
        SECTION("TestTelexEmpty") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                CHECK(L"" == e.Retrieve());
                CHECK(L"" == e.RetrieveRaw());
                CHECK(L"" == e.Peek());
                CHECK(std::size_t{0} == e.Count());
            });
        }
    }

    SECTION("push char") {
        SECTION("TestTelexEmptyPushCharC1_1") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'b'));
                CHECK(L"b" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyPushCharC1_2") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'c'));
                CHECK(L"c" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyPushCharC1_3") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'd'));
                CHECK(L"d" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyPushCharC1_4") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'r'));
                CHECK(L"r" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyPushCharC1_5") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'x'));
                CHECK(L"x" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyPushCharV") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'a'));
                CHECK(L"a" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyCommittedPushChar") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                AssertTelexStatesEqual(TelexStates::Committed, e.PushChar('a'));
                CHECK(L"" == e.Retrieve());
                CHECK(L"" == e.RetrieveRaw());
                CHECK(L"" == e.Peek());
                CHECK(std::size_t{0} == e.Count());
            });
        }
    }

    SECTION("uppercase") {
        SECTION("TestTelexEmptyPushCharUpC1") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'B'));
                CHECK(L"B" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyPushCharUpV") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(L'A'));
                CHECK(L"A" == e.Retrieve());
            });
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
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                CHECK(L"" == e.Retrieve());
                CHECK(L"" == e.RetrieveRaw());
            });
        }

        SECTION("TestTelexBackspaceAfzea") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                if (FeedWord(e, L"afzea") == TelexStates::Valid) {
                    AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                    CHECK(L"ae" == e.Retrieve());
                }
            });
        }
    }

    SECTION("commit") {
        SECTION("TestTelexEmptyCommit") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                CHECK(L"" == e.Retrieve());
            });
        }

        SECTION("TestTelexEmptyCancel") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
                CHECK(L"" == e.RetrieveRaw());
            });
        }

        SECTION("TestTelexEmptyBackconversion") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(std::wstring()));
                CHECK(L"" == e.RetrieveRaw());
            });
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

        SECTION("TestTelexPeekDand") {
            auto config1 = config;
            config1.accept_separate_dd = true;
            MultiConfigTester(config1).Invoke([](auto& e) {
                FeedWord(e, L"dand");
                CHECK(L"\x111\x61n" == e.Peek());
            });
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
            MultiConfigTester(config, 0, 1).Invoke([](auto& e) { UnitTests::TestPeekWord(e, L"OS", L"OSS"); });
        }
        SECTION("TestTelexCapsAAA") {
            TestInvalidWord(L"AA", L"AAA");
        }
        SECTION("TestTelexCapsOWW") {
            TestInvalidWord(L"OW", L"OWW");
        }
        SECTION("TestTelexCapsGIFF") {
            MultiConfigTester(config, 0, 1).Invoke([](auto& e) { UnitTests::TestPeekWord(e, L"GIF", L"GIFF"); });
        }
        SECTION("TestTelexCapsGISS") {
            MultiConfigTester(config, 0, 1).Invoke([](auto& e) { UnitTests::TestPeekWord(e, L"GIS", L"GISS"); });
        }
    }

    SECTION("backspace tests") {
        SECTION("TestTelexBackspaceDdoongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"ddoongf");
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"\x111\x1ed3n" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"\x111\x1ed3" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"\x111" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceLeeen") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"leeen");
                CHECK(L"leen" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
                CHECK(L"lee" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceHuowng") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"huowng");
                CHECK(L"h\x1b0\x1a1ng" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"h\x1b0\x1a1n" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"hu\x1a1" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"hu" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceAoas") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"aoas");
                CHECK(L"aoas" == e.Peek());
                e.Backspace();
                CHECK(L"aoa" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceHeei") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"heei");
                CHECK(L"h\xeai" == e.Peek());
                e.Backspace();
                CHECK(L"h\xea" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceOwa") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"owa");
                CHECK(L"\x1a1\x61" == e.Peek());
                e.Backspace();
                CHECK(L"\x1a1" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceRuowi") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"ruowi");
                CHECK(L"r\x1b0\x1a1i" == e.Peek());
                e.Backspace();
                CHECK(L"ru\x1a1" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceQuee") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"quee");
                CHECK(L"qu\xea" == e.Peek());
                e.Backspace();
                CHECK(L"qu" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceQuys") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"quys");
                CHECK(L"qu\xfd" == e.Peek());
                e.Backspace();
                CHECK(L"qu" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceHieef") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"hieef");
                CHECK(L"hi\x1ec1" == e.Peek());
                e.Backspace();
                CHECK(L"hi" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceGifg") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"gifg");
                CHECK(L"g\xecg" == e.Peek());
                e.Backspace();
                CHECK(L"g\xec" == e.Peek());
                e.Backspace();
                CHECK(L"g" == e.Peek());
                e.PushChar(L'i');
                CHECK(L"gi" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceHofazn") {
            MultiConfigTester(config, 0, 0).Invoke([](auto& e) {
                FeedWord(e, L"hofazn");
                CHECK(L"hoan" == e.Peek());
                e.Backspace();
                CHECK(L"hoa" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceXooong") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"xooong"));
                CHECK(L"xoong" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"xoon" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"xoo" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceThooongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"thooongf"));
                CHECK(L"tho\xf2ng" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"tho\xf2n" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"tho\xf2" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"tho" == e.Peek());
            });
        }

        SECTION("TestTelexBackspaceAssi") {
            auto config1 = config;
            config1.backspaced_word_stays_invalid = false;
            MultiConfigTester(config1, 0, 1).Invoke([](auto& e) {
                FeedWord(e, L"asssi");
                CHECK(L"assi" == e.Peek());
                e.Backspace();
                CHECK(L"ass" == e.Peek());
            });
        }
    }

    SECTION("test tone and w movements") {
        SECTION("TestTelexBackspaceCuwsoc") {
            MultiConfigTester(config, 0, 2).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuwsoc"));
                CHECK(L"c\x1b0\x1edb\x63" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0" == e.Peek());
            });
        }
        SECTION("TestTelexBackspaceCuwocs") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuwocs"));
                CHECK(L"c\x1b0\x1edb\x63" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0" == e.Peek());
            });
        }
        SECTION("TestTelexBackspaceCuocws") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuocws"));
                CHECK(L"c\x1b0\x1edb\x63" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"cu\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"cu" == e.Peek());
            });
        }
        SECTION("TestTelexBackspaceCows") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cows"));
                CHECK(L"c\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c" == e.Peek());
                e.PushChar(L'u');
                CHECK(L"cu" == e.Peek());
            });
        }
        SECTION("TestTelexBackspaceEefsch") {
            MultiConfigTester(config).Invoke([](auto& e) {
                if (FeedWord(e, L"eefsch") == TelexStates::Valid) {
                    AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                    CHECK(L"\x1ebf\x63" == e.Peek());
                    AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                    CHECK(L"\x1ebf" == e.Peek());
                    AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                    CHECK(L"" == e.Peek());
                }
            });
        }
    }

    SECTION("test backconversions") {
        SECTION("TestTelexBackconversionDdoongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"\x111\x1ed3ng"));
                CHECK(L"\x111\x1ed3ng" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionSystem") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Invalid, e.Backconvert(L"system"));
                CHECK(L"system" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionThees") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"TH\x1ebe"));
                CHECK(L"TH\x1ebe" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionVirus") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Invalid, e.Backconvert(L"virus"));
                CHECK(L"virus" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionDdoonfCtrlW") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111\x1ed3nw"));
                CHECK(L"\x111\x1ed3nw" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"\x111\x1ed3n" == e.Peek());
                e.Cancel();
                CHECK(L"\x111\x1ed3n" == e.Peek());
                CHECK(L"\x111\x1ed3n" == e.Retrieve());
            });
        }
        SECTION("TestTelexBackconversionDdCtrlW") {
            MultiConfigTester(config, 0, 3, false).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111w"));
                e.Backspace();
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
            });
        }
        SECTION("TestTelexBackconversionXooong") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"xoong"));
                CHECK(L"xoong" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"xoon" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionXooongUpper") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"XOONG"));
                CHECK(L"XOONG" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionThooongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"tho\xf2ng"));
                CHECK(L"tho\xf2ng" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"tho\xf2n" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionThooongfUpper") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"THO\xd2NG"));
                CHECK(L"THO\xd2NG" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionXoooong") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"x\xf4\xf4ng"));
                CHECK(L"x\xf4\xf4ng" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionXo_oong") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"xo\xf4ng"));
                CHECK(L"xo\xf4ng" == e.Peek());
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backspace());
                CHECK(L"xo\xf4n" == e.Peek());
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backspace());
                CHECK(L"xo\xf4" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"xo" == e.Peek());
            });
        }
        SECTION("TestTelexBackconversionCaays") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"c\x1ea5y"));
                e.Commit();
                CHECK(L"c\x1ea5y" == e.Retrieve());
            });
        }
        SECTION("TestTelexBackconversionQuaays") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"qu\x1ea5y"));
                e.Commit();
                CHECK(L"qu\x1ea5y" == e.Retrieve());
            });
        }
    }

    SECTION("test oa/oe/uy") {
        SECTION("TestTelexTypingOaUy") {
            TestValidWord(L"ho\xe0", L"hoaf");
            TestValidWord(L"ho\xe8", L"hoef");
            TestValidWord(L"lu\x1ef5", L"luyj");
        }
        SECTION("TestTelexTypingOaUyOff") {
            auto config1 = config;
            config1.oa_uy_tone1 = false;
            MultiConfigTester(config1).Invoke([](auto& e) {
                UnitTests::TestValidWord(e, L"h\xf2\x61", L"hoaf");
                UnitTests::TestValidWord(e, L"h\xf2\x65", L"hoef");
                UnitTests::TestValidWord(e, L"l\x1ee5y", L"luyj");
            });
        }
    }

    SECTION("test dd accept") {
        SECTION("TestTelexValidDodongf") {
            TestValidWord(L"\x111\x1ed3ng", L"dodongf");
        }
        SECTION("TestTelexInvalidDodongf") {
            auto config1 = config;
            config1.accept_separate_dd = false;
            MultiConfigTester(config1).Invoke([](auto& e) { UnitTests::TestInvalidWord(e, L"dodongf", L"dodongf"); });
        }
    }

    SECTION("test config oa/uy") {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        MultiConfigTester(config1).Invoke([](auto& e) { UnitTests::TestValidWord(e, L"to\xe0n", L"toanf"); });
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

    SECTION("test doublekey backspace") {
        SECTION("TestTelexBackspaceMooo") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"mooo");
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"mo" == e.Peek());
            });
        }
        SECTION("TestTelexBackspaceMooof") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"mooof");
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"mo" == e.Peek());
            });
        }
    }

    SECTION("autocorrect") {
        SECTION("TestTelexAutocorrectHwuogn") {
            auto config1 = config;
            config1.autocorrect = true;
            MultiConfigTester(config1, 0, 3, false).Invoke([](auto& e) {
                FeedWord(e, L"hwuogn");
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                CHECK(L"h\x1b0\x1a1ng" == e.Retrieve());
            });
        }
        SECTION("TestTelexAutocorrectViets") {
            auto config1 = config;
            config1.autocorrect = true;
            MultiConfigTester(config1, 0, 3, false).Invoke([](auto& e) {
                FeedWord(e, L"viets");
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                CHECK(L"vi\x1ebft" == e.Retrieve());
            });
        }
        SECTION("TestTelexTypingThuowgnf") {
            auto config1 = config;
            config1.autocorrect = true;
            MultiConfigTester(config1, 0, 3, false).Invoke([](auto& e) {
                FeedWord(e, L"thuowgnf");
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                CHECK(L"th\x1b0\x1eddng" == e.Retrieve());
            });
        }
    }
}

} // namespace UnitTests
} // namespace VietType
