// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "stdafx.h"
#include "Telex.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace VietType {
namespace UnitTests {

inline void AssertTelexStatesEqual(VietType::Telex::TelexStates expected, VietType::Telex::TelexStates actual) {
    Assert::AreEqual(static_cast<int>(expected), static_cast<int>(actual));
}

VietType::Telex::TelexStates FeedWord(VietType::Telex::ITelexEngine& e, const wchar_t* input);

void TestValidWord(VietType::Telex::ITelexEngine& e, const wchar_t* expected, const wchar_t* input);

void TestInvalidWord(VietType::Telex::ITelexEngine& e, const wchar_t* expected, const wchar_t* input);

void TestPeekWord(VietType::Telex::ITelexEngine& e, const wchar_t* expected, const wchar_t* input);

} // namespace UnitTests
} // namespace VietType
