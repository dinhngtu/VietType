// VietTypeTests.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include "Telex.h"

#ifndef VIETTYPE_TEST
#error "Cannot build test; not in test mode"
#endif

#ifdef _DEBUG
#define ITERATIONS 10000
#else
#define ITERATIONS 10000000
#endif

using namespace Telex;

int main() {
    TelexConfig c;
    c.oa_uy_tone1 = true;
    TelexEngine tx(c);
    if (tx.push_char(L'n') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'g') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'u') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'y') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'e') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'e') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'n') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L'x') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.push_char(L' ') != TELEX_STATES::COMMITTED) {
        throw std::exception("push_char error");
    }
    std::wstring str = tx.retrieve();
    if (str != L"nguy\x1ec5n ") {
        throw std::exception("retrieve bad string");
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    size_t len = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        tx.reset();
        tx.push_char(L'n');
        tx.push_char(L'g');
        tx.push_char(L'u');
        tx.push_char(L'y');
        tx.push_char(L'e');
        tx.push_char(L'e');
        tx.push_char(L'n');
        tx.push_char(L'x');
        tx.push_char(L' ');
        len += tx.retrieve().length();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::wcout << L"total time: " << ITERATIONS << L" iters = " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;
    std::wcout << L"total len = " << len << std::endl;

    return 0;
}

