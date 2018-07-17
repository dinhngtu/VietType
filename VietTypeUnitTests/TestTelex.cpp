// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "CppUnitTest.h"
#include "Telex.h"
#include "Util.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace VietType::Telex;

namespace VietTypeUnitTests {

static TelexConfig config{
    true,
};

TelexStates FeedWord(TelexEngine& e, wchar_t const *input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
    return e.GetState();
}

void TestValidWord(wchar_t const *expected, wchar_t const *input) {
    TelexEngine e(config);
    e.Reset();
    for (auto c : std::wstring(input)) {
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(c));
    }
    Assert::AreEqual(expected, e.Peek().c_str());
    AssertTelexStatesEqual(TelexStates::COMMITTED, e.Commit());
    Assert::AreEqual(expected, e.Retrieve().c_str());
}

void TestInvalidWord(wchar_t const *expected, wchar_t const *input) {
    TelexEngine e(config);
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
    AssertTelexStatesEqual(TelexStates::COMMITTED_INVALID, e.Commit());
    Assert::AreEqual(expected, e.RetrieveInvalid().c_str());
}

TEST_CLASS(TestTelex) {
public:
    // reset

    TEST_METHOD(TestEmpty) {
        TelexEngine e(config);
        e.Reset();
        Assert::AreEqual(L"", e.Retrieve().c_str());
        Assert::AreEqual(L"", e.RetrieveInvalid().c_str());
        Assert::AreEqual(L"", e.Peek().c_str());
        Assert::AreEqual(std::size_t{ 0 }, e.Count());
    }

    // push char

    TEST_METHOD(TestEmptyPushCharC1_1) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'b'));
        Assert::AreEqual(L"b", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyPushCharC1_2) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'c'));
        Assert::AreEqual(L"c", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyPushCharC1_3) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'd'));
        Assert::AreEqual(L"d", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyPushCharC1_4) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'r'));
        Assert::AreEqual(L"r", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyPushCharC1_5) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'x'));
        Assert::AreEqual(L"x", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyPushCharV) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'a'));
        Assert::AreEqual(L"a", e.Retrieve().c_str());
    }

    // uppercase

    TEST_METHOD(TestEmptyPushCharUpC1) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'B'));
        Assert::AreEqual(L"B", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyPushCharUpV) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(L'A'));
        Assert::AreEqual(L"A", e.Retrieve().c_str());
    }

    // backspace

    TEST_METHOD(TestEmptyBackspace) {
        TelexEngine e(config);
        e.Reset();
        // must leave the class in a valid state
        Assert::AreEqual(L"", e.Retrieve().c_str());
        Assert::AreEqual(L"", e.RetrieveInvalid().c_str());
    }

    // commit

    TEST_METHOD(TestEmptyCommit) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::COMMITTED, e.Commit());
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyForceCommit) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::COMMITTED, e.ForceCommit());
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyCancel) {
        TelexEngine e(config);
        e.Reset();
        AssertTelexStatesEqual(TelexStates::COMMITTED_INVALID, e.Cancel());
        Assert::AreEqual(L"", e.RetrieveInvalid().c_str());
    }

    // word typing tests

    TEST_METHOD(TestTypingDdoongf) {
        TestValidWord(L"\x111\x1ed3ng", L"ddoongf");
    }

    TEST_METHOD(TestTypingAans) {
        TestValidWord(L"\x1ea5n", L"aans");
    }

    TEST_METHOD(TestTypingDdas) {
        TestValidWord(L"\x111\xe1", L"ddas");
    }

    TEST_METHOD(TestTypingNhuwonxg) {
        TestValidWord(L"nh\x1b0\x1ee1ng", L"nhuwonxg");
    }

    TEST_METHOD(TestTypingNguiw) {
        TestValidWord(L"ng\x1b0i", L"nguiw");
    }

    TEST_METHOD(TestTypingNhuwowngx) {
        TestValidWord(L"nh\x1b0\x1ee1ng", L"nhuwowngx");
    }

    TEST_METHOD(TestTypingThoio) {
        TestValidWord(L"th\xf4i", L"thoio");
    }

    // test variations of 'gi'

    TEST_METHOD(TestTypingGif) {
        TestValidWord(L"g\xec", L"gif");
    }

    TEST_METHOD(TestTypingGinf) {
        TestValidWord(L"g\xecn", L"ginf");
    }

    TEST_METHOD(TestTypingGiuowngf) {
        TestValidWord(L"gi\x1b0\x1eddng", L"giuowngf");
    }

    TEST_METHOD(TestTypingGiowf) {
        TestValidWord(L"gi\x1edd", L"giowf");
    }

    TEST_METHOD(TestTypingGiuwax) {
        TestValidWord(L"gi\x1eef""a", L"giuwax");
    }

    // test 'aua' and similar transitions

    TEST_METHOD(TestTypingLauar) {
        TestValidWord(L"l\x1ea9u", L"lauar");
    }

    TEST_METHOD(TestTypingNguayar) {
        TestValidWord(L"ngu\x1ea9y", L"nguayar");
    }

    // peek tests

    TEST_METHOD(TestPeekDd) {
        TelexEngine e(config);
        FeedWord(e, L"dd");
        Assert::AreEqual(L"\x111", e.Peek().c_str());
    }

    // used to cause a crash
    TEST_METHOD(TestPeekZ) {
        TelexEngine e(config);
        FeedWord(e, L"z");
        Assert::AreEqual(L"z", e.Peek().c_str());
    }

    // test peek key ordering

    TEST_METHOD(TestPeekCace) {
        TelexEngine e(config);
        FeedWord(e, L"cace");
        Assert::AreEqual(L"cace", e.Peek().c_str());
    }

    // peek shouldn't crash if tone position is not found
    TEST_METHOD(TestPeekNhaeng) {
        TelexEngine e(config);
        FeedWord(e, L"nhaeng");
        Assert::AreEqual(L"nhaeng", e.Peek().c_str());
    }

    // double key tests

    TEST_METHOD(TestDoubleKeyXuaaan) {
        TestInvalidWord(L"xuaan", L"xuaaan");
    }

    TEST_METHOD(TestDoubleKeyIis) {
        TestInvalidWord(L"iis", L"iis");
    }

    TEST_METHOD(TestDoubleKeyThooongf) {
        TestValidWord(L"tho\xf2ng", L"thooongf");
    }

    TEST_METHOD(TestDoubleKeyThuongz) {
        TestInvalidWord(L"thuongz", L"thuongz");
    }

    TEST_METHOD(TestDoubleKeySystem) {
        TestInvalidWord(L"system", L"system");
    }

    TEST_METHOD(TestNhuwox) {
        TestInvalidWord(L"nhuwox", L"nhuwox");
    }

    // caps

    TEST_METHOD(TestCapsPeekD) {
        TelexEngine e(config);
        e.PushChar(L'D');
        Assert::AreEqual(L"D", e.Peek().c_str());
    }

    TEST_METHOD(TestCapsXuOwngf) {
        TestValidWord(L"X\x1b0\x1edcng", L"XuOwngf");
    }

    TEST_METHOD(TestCapsXuOnWfWx) {
        TestInvalidWord(L"XuOnWfWx", L"XuOnWfWx");
    }

    // backspace tests

    TEST_METHOD(TestBackspaceDdoongf) {
        TelexEngine e(config);
        FeedWord(e, L"ddoongf");
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"\x111\x1ed3n", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"\x111\x1ed3", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"\x111", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"", e.Peek().c_str());
    }

    TEST_METHOD(TestBackspaceMooon) {
        TelexEngine e(config);
        FeedWord(e, L"leeen");
        Assert::AreEqual(L"leen", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::INVALID, e.Backspace());
        Assert::AreEqual(L"lee", e.Peek().c_str());
    }

    TEST_METHOD(TestBackspaceHuowng) {
        TelexEngine e(config);
        FeedWord(e, L"huowng");
        Assert::AreEqual(L"h\x1b0\x1a1ng", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"h\x1b0\x1a1n", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"hu\x1a1", e.Peek().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"hu", e.Peek().c_str());
    }

    // test backconversions

    TEST_METHOD(TestBackconversionDdoongf) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::VALID, e.Backconvert(L"\x111\x1ed3ng"));
        Assert::AreEqual(L"\x111\x1ed3ng", e.Peek().c_str());
    }

    TEST_METHOD(TestBackconversionSystem) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::INVALID, e.Backconvert(L"system"));
        Assert::AreEqual(L"system", e.Peek().c_str());
    }

    TEST_METHOD(TestBackconversionThees) {
        TelexEngine e(config);
        AssertTelexStatesEqual(TelexStates::VALID, e.Backconvert(L"TH\x1ebe"));
        Assert::AreEqual(L"TH\x1ebe", e.Peek().c_str());
    }
};

}
