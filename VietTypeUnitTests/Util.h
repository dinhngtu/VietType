// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace VietType {
namespace Telex {
enum class TelexStates;
}
}

using namespace VietType::Telex;

namespace VietTypeUnitTests {

inline void AssertTelexStatesEqual(TelexStates expected, TelexStates actual) {
    Assert::AreEqual(static_cast<int>(expected), static_cast<int>(actual));
}

}
