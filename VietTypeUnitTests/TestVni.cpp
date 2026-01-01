// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TEST_CASE("TestVni", "[vni]") {
    TelexConfig config{
        .typing_style = TypingStyles::Vni,
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
            TestValidWord(L"\xc2n", L"A6n");
            TestValidWord(L"\x110i", L"D9i");
            TestValidWord(L"\x1ea4n", L"A61n");
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
            TestValidWord(L"\x111\x1ed3ng", L"d9o6ng2");
            TestValidWord(L"\x1ea5n", L"a6n1");
            TestValidWord(L"\x111\xe1", L"d9a1");
            TestValidWord(L"nh\x1b0\x1ee1ng", L"nhu7on4g");
            TestValidWord(L"ng\x1b0i", L"ngui7");
            TestValidWord(L"th\xf4i", L"thoi6");
            TestValidWord(L"n\x1b0\x61", L"nua7");
            TestPeekWord(L"nu\x103", L"nua8");
            TestInvalidWord(L"nua8", L"nua8");
            TestValidWord(L"qu\x1eafm", L"qua8m1");
            TestValidWord(L"qu\xe9t", L"quet1");
            TestValidWord(L"qu\x1ea1u", L"quau5");
            TestValidWord(L"qu\x1ecd", L"quo5");
            TestValidWord(L"qu\x1ecbt", L"quit5");
            TestValidWord(L"qu\xe8o", L"queo2");
            TestValidWord(L"qu\x1edbn", L"quo7n1");
            TestValidWord(L"q\x1b0\x1edbn", L"qu7o7n1");
            TestValidWord(L"qu\x1ee1n", L"quon74");
            TestValidWord(L"\x111\x1b0\x1ee3\x63", L"d9u7o7c5");
            TestValidWord(L"ng\x1b0\x1eddi", L"ngu7oi2");
            TestValidWord(L"thu\x1ebf", L"thue61");
            TestValidWord(L"khu\x1eafng", L"khua8ng1");
            TestValidWord(L"khu\x1eafng", L"khua81ng");
            TestValidWord(L"ra", L"ra");
        }

        SECTION("test variations of 'gi'") {
            TestValidWord(L"g\xec", L"gi2");
            TestValidWord(L"g\xecn", L"gin2");
            TestValidWord(L"gi\x1b0\x1eddng", L"giuo7ng2");
            TestValidWord(L"gi\x1edd", L"gio72");
            TestValidWord(L"gi\x1eef\x61", L"giu7a4");
            TestValidWord(L"gi\x169", L"giu4");
            TestValidWord(L"giu\x1ed9\x63", L"giuo6c5");
            TestValidWord(L"gi\xe8m", L"giem2");
            TestValidWord(L"gi\xea", L"gie6");
        }

        SECTION("test 'aua' and similar transitions") {
            TestValidWord(L"l\x1ea9u", L"lau63");
            TestValidWord(L"ngu\x1ea9y", L"nguay63");
            TestValidWord(L"l\x1b0u", L"luu7");
            TestValidWord(L"h\x1b0\x1a1u", L"huou7");
        }

        SECTION("irregular") {
            TestValidWord(L"qu\x1edbi", L"quoi71");
            TestValidWord(L"\x111\x1eafk", L"d9a8k1");
        }

        SECTION("peek tests") {
            TestPeekWord(L"\x111", L"d9");
            TestPeekWord(L"d9", L"d99");
            TestPeekWord(L"ad", L"ad");
            TestPeekWord(L"qu\x103", L"qua8");
            TestPeekWord(L"xo6", L"xo66");
            TestPeekWord(L"z", L"z");
            TestPeekWord(L"ca3c", L"ca3c");
            TestPeekWord(L"cace", L"cace");
            TestPeekWord(L"nhaeng", L"nhaeng");
        }

        SECTION("double key tests") {
            TestInvalidWord(L"xua6n", L"xua66n");
            TestInvalidWord(L"iis", L"iis");
            TestInvalidWord(L"thuong0", L"thuong0");
            TestInvalidWord(L"nhu7o4", L"nhu7o4");
            TestInvalidWord(L"to6l", L"to66l");
            TestInvalidWord(L"cu7o7c5", L"cu7o77c5");
            TestInvalidWord(L"qu7o7n1", L"qu7o77n1");
            TestValidWord(L"c\x1ea5y", L"ca61y");
        }

        SECTION("caps") {
            TestPeekWord(L"D", L"D");
            TestValidWord(L"X\x1b0\x1edcng", L"XuO7ng2");
            TestInvalidWord(L"XuOn7f74", L"XuOn7f74");
            TestValidWord(L"NG\x1af\x1edCI", L"NGUOI72");
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
                if (config.optimize_multilang <= 2) {
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
                if (!config.autocorrect) {
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
            TestValidWord(L"to\xe0n", L"toan2");
            if (config.oa_uy_tone1) {
                TestValidWord(L"ho\xe0", L"hoa2");
                TestValidWord(L"ho\xe8", L"hoe2");
                TestValidWord(L"lu\x1ef5", L"luy5");
            } else {
                TestValidWord(L"h\xf2\x61", L"hoa2");
                TestValidWord(L"h\xf2\x65", L"hoe2");
                TestValidWord(L"l\x1ee5y", L"luy5");
            }
        }

        SECTION("test dd accept") {
            if (config.accept_separate_dd) {
                SECTION("TestVniPeekDand") {
                    FeedWord(*engine, L"dan9");
                    CHECK(L"\x111\x61n" == engine->Peek());
                }
                SECTION("TestVniValidDodongf") {
                    TestValidWord(L"\x111\x1ed3ng", L"do96ng2");
                }
            } else {
                TestInvalidWord(L"do96ng2", L"do96ng2");
            }
        }

        SECTION("test doublekey backspace") {
            SECTION("TestVniBackspaceMooo") {
                FeedWord(*engine, L"mo66");
                CHECK(L"mo6" == engine->Peek());
                if (config.backspaced_word_stays_invalid)
                    AssertTelexStatesEqual(TelexStates::Invalid, engine->Backspace());
                else
                    AssertTelexStatesEqual(TelexStates::Valid, engine->Backspace());
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
            if (config.autocorrect) {
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
