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

void FeedWord(TelexEngine& e, wchar_t const *input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
}

void TestValidWord(wchar_t const *expected, wchar_t const *input) {
    TelexEngine e(config);
    e.Reset();
    for (auto c : std::wstring(input)) {
        AssertTelexStatesEqual(TelexStates::VALID, e.PushChar(c));
    }
    AssertTelexStatesEqual(TelexStates::COMMITTED, e.Commit());
    Assert::AreEqual(expected, e.Retrieve().c_str());
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
        e.Commit();
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyForceCommit) {
        TelexEngine e(config);
        e.Reset();
        e.ForceCommit();
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }

    TEST_METHOD(TestEmptyCancel) {
        TelexEngine e(config);
        e.Reset();
        e.Cancel();
        Assert::AreEqual(L"", e.RetrieveInvalid().c_str());
    }

    // word typing tests

    TEST_METHOD(TestTyping1) {
        TestValidWord(L"\u0111\u1ED3ng", L"ddoongf");
    }

    TEST_METHOD(TestTyping2) {
        TestValidWord(L"\u1EA5n", L"aans");
    }

    TEST_METHOD(TestTyping3) {
        TestValidWord(L"\u0111\u00E1", L"dads");
    }

    // backspace tests

    TEST_METHOD(TestBackspace1) {
        TelexEngine e(config);
        FeedWord(e, L"ddoongf");
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"\u0111\u1ED3n", e.Retrieve().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"\u0111\u1ED3", e.Retrieve().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"\u0111", e.Retrieve().c_str());
        AssertTelexStatesEqual(TelexStates::VALID, e.Backspace());
        Assert::AreEqual(L"", e.Retrieve().c_str());
    }
};

}
