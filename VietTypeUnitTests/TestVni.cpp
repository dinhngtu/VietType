// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestVni", "[vni]") {
    auto level = GENERATE(0, 1, 2, 3);
    auto autocorrect = GENERATE(true, false);

    TelexConfig config{.typing_style = TypingStyles::Vni};
    config.optimize_multilang = level;
    config.autocorrect = autocorrect;

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
        SECTION("TestVniEmpty") {
            engine->Reset();
            CHECK(L"" == engine->Retrieve());
            CHECK(L"" == engine->RetrieveRaw());
            CHECK(L"" == engine->Peek());
            CHECK(std::size_t{0} == engine->Count());
        }
    }

    SECTION("push char") {
        SECTION("TestVniEmptyPushCharDigits") {
            for (wchar_t i = L'0'; i <= L'9'; i++) {
                engine->Reset();
                AssertTelexStatesEqual(TelexStates::Invalid, engine->PushChar(i));
                auto compare = std::wstring(1, i);
                CHECK(compare == engine->Retrieve());
            }
        }

        SECTION("TestVniEmptyCommittedPushChar") {
            engine->Reset();
            AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
            AssertTelexStatesEqual(TelexStates::Committed, engine->PushChar('a'));
            CHECK(L"" == engine->Retrieve());
            CHECK(L"" == engine->RetrieveRaw());
            CHECK(L"" == engine->Peek());
            CHECK(std::size_t{0} == engine->Count());
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
                engine->Reset();
                CHECK(L"" == engine->Retrieve());
                CHECK(L"" == engine->RetrieveRaw());
            }
        }

        SECTION("commit") {
            SECTION("TestVniEmptyCommit") {
                engine->Reset();
                AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
                CHECK(L"" == engine->Retrieve());
            }

            SECTION("TestVniEmptyCancel") {
                engine->Reset();
                AssertTelexStatesEqual(TelexStates::CommittedInvalid, engine->Cancel());
                CHECK(L"" == engine->RetrieveRaw());
            }

            SECTION("TestVniEmptyBackconversion") {
                engine->Reset();
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(std::wstring()));
                CHECK(L"" == engine->RetrieveRaw());
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
                auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));
                FeedWord(*e, L"dan9");
                CHECK(L"\x111\x61n" == e->Peek());
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
                FeedWord(*engine, L"d9o6ng2");
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"\x111\x1ed3n" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"\x111\x1ed3" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"\x111" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"" == engine->Peek());
            }

            SECTION("TestVniBackspaceLeeen") {
                FeedWord(*engine, L"le66n");
                CHECK(L"le6n" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, engine->Backspace());
                CHECK(L"le6" == engine->Peek());
            }

            SECTION("TestVniBackspaceHuowng") {
                FeedWord(*engine, L"huo7ng");
                CHECK(L"h\x1b0\x1a1ng" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"h\x1b0\x1a1n" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"hu\x1a1" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"hu" == engine->Peek());
            }

            SECTION("TestVniBackspaceAoas") {
                FeedWord(*engine, L"ao61");
                CHECK(L"ao61" == engine->Peek());
                engine->Backspace();
                CHECK(L"ao6" == engine->Peek());
            }

            SECTION("TestVniBackspaceHeei") {
                FeedWord(*engine, L"he6i");
                CHECK(L"h\xeai" == engine->Peek());
                engine->Backspace();
                CHECK(L"h\xea" == engine->Peek());
            }

            SECTION("TestVniBackspaceOwa") {
                FeedWord(*engine, L"o7a");
                CHECK(L"\x1a1\x61" == engine->Peek());
                engine->Backspace();
                CHECK(L"\x1a1" == engine->Peek());
            }

            SECTION("TestVniBackspaceRuowi") {
                FeedWord(*engine, L"ruo7i");
                CHECK(L"r\x1b0\x1a1i" == engine->Peek());
                engine->Backspace();
                CHECK(L"ru\x1a1" == engine->Peek());
            }

            SECTION("TestVniBackspaceQuee") {
                FeedWord(*engine, L"que6");
                CHECK(L"qu\xea" == engine->Peek());
                engine->Backspace();
                CHECK(L"qu" == engine->Peek());
            }

            SECTION("TestVniBackspaceQuys") {
                FeedWord(*engine, L"quy1");
                CHECK(L"qu\xfd" == engine->Peek());
                engine->Backspace();
                CHECK(L"qu" == engine->Peek());
            }

            SECTION("TestVniBackspaceHieef") {
                FeedWord(*engine, L"hie62");
                CHECK(L"hi\x1ec1" == engine->Peek());
                engine->Backspace();
                CHECK(L"hi" == engine->Peek());
            }

            SECTION("TestVniBackspaceGifg") {
                FeedWord(*engine, L"gi2g");
                CHECK(L"g\xecg" == engine->Peek());
                engine->Backspace();
                CHECK(L"g\xec" == engine->Peek());
            }
        }

        SECTION("test tone and w movements") {
            SECTION("TestVniBackspaceCuwsoc") {
                if (level <= 2) {
                    AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cu71oc"));
                    CHECK(L"c\x1b0\x1edb\x63" == engine->Peek());
                    AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                    CHECK(L"c\x1b0\x1edb" == engine->Peek());
                    AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                    CHECK(L"c\x1b0" == engine->Peek());
                }
            }
            SECTION("TestVniBackspaceCuwocs") {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cu7oc1"));
                CHECK(L"c\x1b0\x1edb\x63" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"c\x1b0\x1edb" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"c\x1b0" == engine->Peek());
            }
            SECTION("TestVniBackspaceCuocws") {
                AssertTelexStatesEqual(TelexStates::Valid, FeedWord(*engine, L"cuoc71"));
                CHECK(L"c\x1b0\x1edb\x63" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"cu\x1edb" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"cu" == engine->Peek());
            }
        }

        SECTION("test backconversions") {
            SECTION("TestVniBackconversionDdoongf") {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"\x111\x1ed3ng"));
                CHECK(L"\x111\x1ed3ng" == engine->Peek());
            }
            SECTION("TestVniBackconversionThees") {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"TH\x1ebe"));
                CHECK(L"TH\x1ebe" == engine->Peek());
            }
            SECTION("TestVniBackconversionDdoonfCtrlW") {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"\x111\x1ed3n7"));
                CHECK(L"\x111\x1ed3n7" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"\x111\x1ed3n" == engine->Peek());
                engine->Cancel();
                CHECK(L"\x111\x1ed3n" == engine->Peek());
                CHECK(L"\x111\x1ed3n" == engine->Retrieve());
            }
            SECTION("TestVniBackconversionDdCtrlW") {
                if (!autocorrect) {
                    AssertTelexStatesEqual(
                        TelexStates::BackconvertFailed,
                        engine->Backconvert(
                            L"\x111"
                            L"7"));
                    engine->Backspace();
                    AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                    AssertTelexStatesEqual(TelexStates::CommittedInvalid, engine->Cancel());
                }
            }
            SECTION("TestVniBackconversionThooongf") {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"tho\xf2ng"));
                CHECK(L"tho\xf2ng" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"tho\xf2n" == engine->Peek());
            }

            SECTION("TestVniBackconversionThooongfUpper") {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"THO\xd2NG"));
                CHECK(L"THO\xd2NG" == engine->Peek());
            }

            SECTION("TestVniBackconversionXoooong") {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"x\xf4\xf4ng"));
                CHECK(L"x\xf4\xf4ng" == engine->Peek());
            }

            SECTION("TestVniBackconversionXo_oong") {
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backconvert(L"xo\xf4ng"));
                CHECK(L"xo\xf4ng" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backspace());
                CHECK(L"xo\xf4n" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::BackconvertFailed, engine->Backspace());
                CHECK(L"xo\xf4" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
                CHECK(L"xo" == engine->Peek());
            }

            SECTION("TestVniBackconversionCaays") {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"c\x1ea5y"));
                engine->Commit();
                CHECK(L"c\x1ea5y" == engine->Retrieve());
            }

            SECTION("TestVniBackconversionQuaays") {
                AssertTelexStatesEqual(TelexStates::Valid, engine->Backconvert(L"qu\x1ea5y"));
                engine->Commit();
                CHECK(L"qu\x1ea5y" == engine->Retrieve());
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
                auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));
                UnitTests::TestValidWord(*e, L"h\xf2\x61", L"hoa2");
                UnitTests::TestValidWord(*e, L"h\xf2\x65", L"hoe2");
                UnitTests::TestValidWord(*e, L"l\x1ee5y", L"luy5");
            }
        }

        SECTION("test dd accept") {
            SECTION("TestVniValidDodongf") {
                TestValidWord(L"\x111\x1ed3ng", L"do96ng2");
            }
            SECTION("TestVniInvalidDodongf") {
                auto config1 = config;
                config1.accept_separate_dd = false;
                auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));
                UnitTests::TestInvalidWord(*e, L"dodongf", L"dodongf");
            }
        }

        SECTION("test config oa/uy") {
            auto config1 = config;
            config1.oa_uy_tone1 = false;
            auto e = std::unique_ptr<ITelexEngine>(TelexNew(config1));
            UnitTests::TestValidWord(*e, L"to\xe0n", L"toan2");
        }

        SECTION("test doublekey backspace") {
            SECTION("TestVniBackspaceMooo") {
                FeedWord(*engine, L"mo66");
                CHECK(L"mo6" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, engine->Backspace());
                CHECK(L"mo" == engine->Peek());
            }
            SECTION("TestVniBackspaceMooof") {
                FeedWord(*engine, L"mo662");
                CHECK(L"mo62" == engine->Peek());
                AssertTelexStatesEqual(TelexStates::Invalid, engine->Backspace());
                CHECK(L"mo6" == engine->Peek());
            }
        }

        SECTION("autocorrect") {
            if (autocorrect) {
                SECTION("TestVniAutocorrectViet1") {
                    FeedWord(*engine, L"viet1");
                    AssertTelexStatesEqual(TelexStates::Committed, engine->Commit());
                    CHECK(L"vi\x1ebft" == engine->Retrieve());
                }
            }
        }
    }
}

} // namespace UnitTests
} // namespace VietType
