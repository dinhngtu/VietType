// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "Util.h"
#include "Telex.h"

using namespace VietType::Telex;

namespace VietType {
namespace UnitTests {

TelexStates FeedWord(ITelexEngine& e, const wchar_t* input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
    return e.GetState();
}

void TestValidWord(ITelexEngine& e, const wchar_t* expected, const wchar_t* input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        AssertTelexStatesEqual(TelexStates::Valid, e.PushChar(c));
    }
    CHECK(std::wstring(expected) == e.Peek());
    AssertTelexStatesEqual(TelexStates::Committed, e.Commit());
    CHECK(std::wstring(expected) == e.Retrieve());
}

void TestInvalidWord(ITelexEngine& e, const wchar_t* expected, const wchar_t* input) {
    e.Reset();
    for (auto c : std::wstring(input)) {
        e.PushChar(c);
    }
    AssertTelexStatesEqual(TelexStates::CommittedInvalid, e.Commit());
    CHECK(std::wstring(expected) == e.RetrieveRaw());
}

void TestPeekWord(ITelexEngine& e, const wchar_t* expected, const wchar_t* input) {
    FeedWord(e, input);
    CHECK(std::wstring(expected) == e.Peek());
}

} // namespace UnitTests
} // namespace VietType
