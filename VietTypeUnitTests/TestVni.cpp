// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestVni", "[vni]") {
    const TelexConfig config{.typing_style = TypingStyles::Vni};

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
        SECTION("TestVniEmpty") {
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
        SECTION("TestVniEmptyPushCharDigits") {
            MultiConfigTester(config).Invoke([](auto& e) {
                for (wchar_t i = L'0'; i <= L'9'; i++) {
                    e.Reset();
                    AssertTelexStatesEqual(TelexStates::Invalid, e.PushChar(i));
                    auto compare = std::wstring(1, i);
                    CHECK(compare == e.Retrieve());
                }
            });
        }

        SECTION("TestVniEmptyCommittedPushChar") {
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
        SECTION("TestVniTypingUpAai_1") {
            TestValidWord(L"\xc2n", L"A6n");
        }
        SECTION("TestVniTypingUpDdi_1") {
            TestValidWord(L"\x110i", L"D9i");
        }
        SECTION("TestVniTypingUpAasn_1") {
            TestValidWord(L"\x1ea4n", L"A61n");
        }
    }

    SECTION("backspace") {
        SECTION("TestVniEmptyBackspace") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                CHECK(L"" == e.Retrieve());
                CHECK(L"" == e.RetrieveRaw());
            });
        }
    }

    SECTION("commit") {
        SECTION("TestVniEmptyCommit") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                CHECK(L"" == e.Retrieve());
            });
        }

        SECTION("TestVniEmptyCancel") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
                CHECK(L"" == e.RetrieveRaw());
            });
        }

        SECTION("TestVniEmptyBackconversion") {
            MultiConfigTester(config).Invoke([](auto& e) {
                e.Reset();
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(std::wstring()));
                CHECK(L"" == e.RetrieveRaw());
            });
        }
    }

    SECTION("word typing tests") {
        SECTION("TestVniTypingDdoongf") {
            TestValidWord(L"\x111\x1ed3ng", L"d9o6ng2");
        }
        SECTION("TestVniTypingAans") {
            TestValidWord(L"\x1ea5n", L"a6n1");
        }
        SECTION("TestVniTypingDdas") {
            TestValidWord(L"\x111\xe1", L"d9a1");
        }
        SECTION("TestVniTypingNhuwonxg") {
            TestValidWord(L"nh\x1b0\x1ee1ng", L"nhu7on4g");
        }
        SECTION("TestVniTypingNguiw") {
            TestValidWord(L"ng\x1b0i", L"ngui7");
        }
        SECTION("TestVniTypingThoio") {
            TestValidWord(L"th\xf4i", L"thoi6");
        }
        SECTION("TestVniTypingNua7") {
            TestValidWord(L"n\x1b0\x61", L"nua7");
        }
        SECTION("TestVniTypingNua8") {
            TestPeekWord(L"nu\x103", L"nua8");
            TestInvalidWord(L"nua8", L"nua8");
        }
        SECTION("TestVniTypingQuawms") {
            TestValidWord(L"qu\x1eafm", L"qua8m1");
        }
        SECTION("TestVniTypingQuets") {
            TestValidWord(L"qu\xe9t", L"quet1");
        }
        SECTION("TestVniTypingQuauj") {
            TestValidWord(L"qu\x1ea1u", L"quau5");
        }
        SECTION("TestVniTypingQuoj") {
            TestValidWord(L"qu\x1ecd", L"quo5");
        }
        SECTION("TestVniTypingQuitj") {
            TestValidWord(L"qu\x1ecbt", L"quit5");
        }
        SECTION("TestVniTypingQueof") {
            TestValidWord(L"qu\xe8o", L"queo2");
        }
        SECTION("TestVniTypingQuowns") {
            TestValidWord(L"qu\x1edbn", L"quo7n1");
        }
        SECTION("TestVniTypingQuwowns") {
            TestValidWord(L"q\x1b0\x1edbn", L"qu7o7n1");
        }
        SECTION("TestVniTypingQuonwx") {
            TestValidWord(L"qu\x1ee1n", L"quon74");
        }
        SECTION("TestVniTypingDduwowcj") {
            TestValidWord(L"\x111\x1b0\x1ee3\x63", L"d9u7o7c5");
        }
        SECTION("TestVniTypingNguwoif") {
            TestValidWord(L"ng\x1b0\x1eddi", L"ngu7oi2");
        }
        SECTION("TestVniTypingThuees") {
            TestValidWord(L"thu\x1ebf", L"thue61");
        }
        SECTION("TestVniTypingKhuawngs") {
            TestValidWord(L"khu\x1eafng", L"khua8ng1");
        }
        SECTION("TestVniTypingKhuawsng") {
            TestValidWord(L"khu\x1eafng", L"khua81ng");
        }
        SECTION("TestVniTypingRa") {
            TestValidWord(L"ra", L"ra");
        }
    }

    SECTION("test variations of 'gi'") {
        SECTION("TestVniTypingGif") {
            TestValidWord(L"g\xec", L"gi2");
        }
        SECTION("TestVniTypingGinf") {
            TestValidWord(L"g\xecn", L"gin2");
        }
        SECTION("TestVniTypingGiuowngf") {
            TestValidWord(L"gi\x1b0\x1eddng", L"giuo7ng2");
        }
        SECTION("TestVniTypingGiowf") {
            TestValidWord(L"gi\x1edd", L"gio72");
        }
        SECTION("TestVniTypingGiuwax") {
            TestValidWord(L"gi\x1eef\x61", L"giu7a4");
        }
        SECTION("TestVniTypingGiux") {
            TestValidWord(L"gi\x169", L"giu4");
        }
        SECTION("TestVniTypingGiuoocj") {
            TestValidWord(L"giu\x1ed9\x63", L"giuo6c5");
        }
        SECTION("TestVniTypingGiemf") {
            TestValidWord(L"gi\xe8m", L"giem2");
        }
        SECTION("TestVniTypingGiee") {
            TestValidWord(L"gi\xea", L"gie6");
        }
    }

    SECTION("test 'aua' and similar transitions") {
        SECTION("TestVniTypingLauar") {
            TestValidWord(L"l\x1ea9u", L"lau63");
        }
        SECTION("TestVniTypingNguayar") {
            TestValidWord(L"ngu\x1ea9y", L"nguay63");
        }
        SECTION("TestVniTypingLuuw") {
            TestValidWord(L"l\x1b0u", L"luu7");
        }
        SECTION("TestVniTypingHuouw") {
            TestValidWord(L"h\x1b0\x1a1u", L"huou7");
        }
    }

    SECTION("irregular") {
        SECTION("TestVniTypingQuoiws") {
            TestValidWord(L"qu\x1edbi", L"quoi71");
        }
        SECTION("TestVniTypingDawks") {
            TestValidWord(L"\x111\x1eafk", L"d9a8k1");
        }
    }

    SECTION("peek tests") {
        SECTION("TestVniPeekDd") {
            TestPeekWord(L"\x111", L"d9");
        }
        SECTION("TestVniPeekDdd") {
            TestPeekWord(L"d9", L"d99");
        }
        SECTION("TestVniPeekAd") {
            TestPeekWord(L"ad", L"ad");
        }
        SECTION("TestVniPeekQuaw") {
            TestPeekWord(L"qu\x103", L"qua8");
        }
        SECTION("TestVniPeekXo66") {
            TestPeekWord(L"xo6", L"xo66");
        }
        SECTION("TestVniPeekZ") {
            TestPeekWord(L"z", L"z");
        }
        SECTION("TestVniPeekCarc") {
            TestPeekWord(L"ca3c", L"ca3c");
        }
        SECTION("TestVniPeekCace") {
            TestPeekWord(L"cace", L"cace");
        }
        SECTION("TestVniPeekNhaeng") {
            TestPeekWord(L"nhaeng", L"nhaeng");
        }

        SECTION("TestVniPeekDand") {
            auto config1 = config;
            config1.accept_separate_dd = true;
            MultiConfigTester(config1).Invoke([](auto& e) {
                FeedWord(e, L"dan9");
                CHECK(L"\x111\x61n" == e.Peek());
            });
        }
    }

    SECTION("double key tests") {
        SECTION("TestVniDoubleKeyXuaaan") {
            TestInvalidWord(L"xua6n", L"xua66n");
        }
        SECTION("TestVniDoubleKeyIis") {
            TestInvalidWord(L"iis", L"iis");
        }
        SECTION("TestVniDoubleKeyThuongz") {
            TestInvalidWord(L"thuong0", L"thuong0");
        }
        SECTION("TestVniNhuwox") {
            TestInvalidWord(L"nhu7o4", L"nhu7o4");
        }
        SECTION("TestVniToool") {
            TestInvalidWord(L"to6l", L"to66l");
        }
        SECTION("TestVniCuwowwcj") {
            TestInvalidWord(L"cu7o7c5", L"cu7o77c5");
        }
        SECTION("TestVniTypingQuwowwns") {
            TestInvalidWord(L"qu7o7n1", L"qu7o77n1");
        }
        SECTION("TestVniTypingCaasy") {
            TestValidWord(L"c\x1ea5y", L"ca61y");
        }
    }

    SECTION("caps") {
        SECTION("TestVniCapsPeekD") {
            TestPeekWord(L"D", L"D");
        }
        SECTION("TestVniCapsXuOwngf") {
            TestValidWord(L"X\x1b0\x1edcng", L"XuO7ng2");
        }
        SECTION("TestVniCapsXuOnWfWx") {
            TestInvalidWord(L"XuOn7f74", L"XuOn7f74");
        }
        SECTION("TestVniAllCapsNguoiwf") {
            TestValidWord(L"NG\x1af\x1edCI", L"NGUOI72");
        }
    }

    SECTION("backspace tests") {
        SECTION("TestVniBackspaceDdoongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"d9o6ng2");
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

        SECTION("TestVniBackspaceLeeen") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"le66n");
                CHECK(L"le6n" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
                CHECK(L"le6" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceHuowng") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"huo7ng");
                CHECK(L"h\x1b0\x1a1ng" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"h\x1b0\x1a1n" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"hu\x1a1" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"hu" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceAoas") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"ao61");
                CHECK(L"ao61" == e.Peek());
                e.Backspace();
                CHECK(L"ao6" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceHeei") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"he6i");
                CHECK(L"h\xeai" == e.Peek());
                e.Backspace();
                CHECK(L"h\xea" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceOwa") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"o7a");
                CHECK(L"\x1a1\x61" == e.Peek());
                e.Backspace();
                CHECK(L"\x1a1" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceRuowi") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"ruo7i");
                CHECK(L"r\x1b0\x1a1i" == e.Peek());
                e.Backspace();
                CHECK(L"ru\x1a1" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceQuee") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"que6");
                CHECK(L"qu\xea" == e.Peek());
                e.Backspace();
                CHECK(L"qu" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceQuys") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"quy1");
                CHECK(L"qu\xfd" == e.Peek());
                e.Backspace();
                CHECK(L"qu" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceHieef") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"hie62");
                CHECK(L"hi\x1ec1" == e.Peek());
                e.Backspace();
                CHECK(L"hi" == e.Peek());
            });
        }

        SECTION("TestVniBackspaceGifg") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"gi2g");
                CHECK(L"g\xecg" == e.Peek());
                e.Backspace();
                CHECK(L"g\xec" == e.Peek());
            });
        }
    }

    SECTION("test tone and w movements") {
        SECTION("TestVniBackspaceCuwsoc") {
            MultiConfigTester(config, 0, 2).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cu71oc"));
                CHECK(L"c\x1b0\x1edb\x63" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0" == e.Peek());
            });
        }
        SECTION("TestVniBackspaceCuwocs") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cu7oc1"));
                CHECK(L"c\x1b0\x1edb\x63" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"c\x1b0" == e.Peek());
            });
        }
        SECTION("TestVniBackspaceCuocws") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(e, L"cuoc71"));
                CHECK(L"c\x1b0\x1edb\x63" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"cu\x1edb" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"cu" == e.Peek());
            });
        }
    }

    SECTION("test backconversions") {
        SECTION("TestVniBackconversionDdoongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"\x111\x1ed3ng"));
                CHECK(L"\x111\x1ed3ng" == e.Peek());
            });
        }
        SECTION("TestVniBackconversionThees") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"TH\x1ebe"));
                CHECK(L"TH\x1ebe" == e.Peek());
            });
        }
        SECTION("TestVniBackconversionDdoonfCtrlW") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"\x111\x1ed3n7"));
                CHECK(L"\x111\x1ed3n7" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"\x111\x1ed3n" == e.Peek());
                e.Cancel();
                CHECK(L"\x111\x1ed3n" == e.Peek());
                CHECK(L"\x111\x1ed3n" == e.Retrieve());
            });
        }
        SECTION("TestVniBackconversionDdCtrlW") {
            MultiConfigTester(config, 0, 3, false).Invoke([](auto& e) {
                AssertTelexStatesEqual(
                    TelexStates::BackconvertFailed,
                    e.Backconvert(
                        L"\x111"
                        L"7"));
                e.Backspace();
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Cancel());
            });
        }
        SECTION("TestVniBackconversionThooongf") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"tho\xf2ng"));
                CHECK(L"tho\xf2ng" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Valid, e.Backspace());
                CHECK(L"tho\xf2n" == e.Peek());
            });
        }
        SECTION("TestVniBackconversionThooongfUpper") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"THO\xd2NG"));
                CHECK(L"THO\xd2NG" == e.Peek());
            });
        }
        SECTION("TestVniBackconversionXoooong") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, e.Backconvert(L"x\xf4\xf4ng"));
                CHECK(L"x\xf4\xf4ng" == e.Peek());
            });
        }
        SECTION("TestVniBackconversionXo_oong") {
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
        SECTION("TestVniBackconversionCaays") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"c\x1ea5y"));
                e.Commit();
                CHECK(L"c\x1ea5y" == e.Retrieve());
            });
        }
        SECTION("TestVniBackconversionQuaays") {
            MultiConfigTester(config).Invoke([](auto& e) {
                AssertTelexStatesEqual(TelexStates::Valid, e.Backconvert(L"qu\x1ea5y"));
                e.Commit();
                CHECK(L"qu\x1ea5y" == e.Retrieve());
            });
        }
    }

    SECTION("test oa/oe/uy") {
        SECTION("TestVniTypingOaUy") {
            TestValidWord(L"ho\xe0", L"hoa2");
            TestValidWord(L"ho\xe8", L"hoe2");
            TestValidWord(L"lu\x1ef5", L"luy5");
        }
        SECTION("TestVniTypingOaUyOff") {
            auto config1 = config;
            config1.oa_uy_tone1 = false;
            MultiConfigTester(config1).Invoke([](auto& e) {
                UnitTests::TestValidWord(e, L"h\xf2\x61", L"hoa2");
                UnitTests::TestValidWord(e, L"h\xf2\x65", L"hoe2");
                UnitTests::TestValidWord(e, L"l\x1ee5y", L"luy5");
            });
        }
    }

    SECTION("test dd accept") {
        SECTION("TestVniValidDodongf") {
            TestValidWord(L"\x111\x1ed3ng", L"do96ng2");
        }
        SECTION("TestVniInvalidDodongf") {
            auto config1 = config;
            config1.accept_separate_dd = false;
            MultiConfigTester(config1).Invoke([](auto& e) { UnitTests::TestInvalidWord(e, L"dodongf", L"dodongf"); });
        }
    }

    SECTION("test config oa/uy") {
        auto config1 = config;
        config1.oa_uy_tone1 = false;
        MultiConfigTester(config1).Invoke([](auto& e) { UnitTests::TestValidWord(e, L"to\xe0n", L"toan2"); });
    }

    SECTION("test doublekey backspace") {
        SECTION("TestVniBackspaceMooo") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"mo66");
                CHECK(L"mo6" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
                CHECK(L"mo" == e.Peek());
            });
        }
        SECTION("TestVniBackspaceMooof") {
            MultiConfigTester(config).Invoke([](auto& e) {
                FeedWord(e, L"mo662");
                CHECK(L"mo62" == e.Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, e.Backspace());
                CHECK(L"mo6" == e.Peek());
            });
        }
    }

    SECTION("autocorrect") {
        SECTION("TestVniAutocorrectViet1") {
            auto config1 = config;
            config1.autocorrect = true;
            MultiConfigTester(config1, 0, 3, false).Invoke([](auto& e) {
                FeedWord(e, L"viet1");
                AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
                CHECK(L"vi\x1ebft" == e.Retrieve());
            });
        }
    }
}

} // namespace UnitTests
} // namespace VietType
