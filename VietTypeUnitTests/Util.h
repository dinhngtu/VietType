// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <functional>
#include <memory>
#include "Telex.h"
#include "catch_amalgamated.hpp"

namespace VietType {
namespace UnitTests {

inline void AssertTelexStatesEqual(VietType::Telex::TelexStates expected, VietType::Telex::TelexStates actual) {
    CHECK(static_cast<int>(expected) == static_cast<int>(actual));
}

VietType::Telex::TelexStates FeedWord(VietType::Telex::ITelexEngine& e, const wchar_t* input);

void TestValidWord(VietType::Telex::ITelexEngine& e, const wchar_t* expected, const wchar_t* input);

void TestInvalidWord(VietType::Telex::ITelexEngine& e, const wchar_t* expected, const wchar_t* input);

void TestPeekWord(VietType::Telex::ITelexEngine& e, const wchar_t* expected, const wchar_t* input);

class MultiConfigTester {
public:
    MultiConfigTester(
        const VietType::Telex::TelexConfig& config,
        int optimizeMultilangMin = 0,
        int optimizeMultilangMax = 3,
        bool testAutocorrect = true)
        : _config(config), _omMin(optimizeMultilangMin), _omMax(optimizeMultilangMax), _ac(testAutocorrect) {
    }

    void Invoke(std::function<void(VietType::Telex::ITelexEngine&)> f) const {
        for (int level = _omMin; level <= _omMax; level++) {
#ifdef _DEBUG
            UNSCOPED_INFO("level " << level);
#endif
            for (int autocorrect = _ac ? 0 : 1; autocorrect <= 1; autocorrect++) {
#ifdef _DEBUG
                UNSCOPED_INFO("autocorrect " << autocorrect);
#endif
                auto config = _config;
                config.optimize_multilang = level;
                if (_ac)
                    config.autocorrect = !!autocorrect;
                std::unique_ptr<VietType::Telex::ITelexEngine> e(VietType::Telex::TelexNew(config));
                f(*e);
            }
        }
    }

private:
    VietType::Telex::TelexConfig _config;
    int _omMin;
    int _omMax;
    bool _ac;
};

} // namespace UnitTests
} // namespace VietType
