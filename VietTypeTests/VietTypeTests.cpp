// VietTypeTests.cpp : Defines the entry point for the console application.
//

#include <cassert>
#include <iostream>
#include <chrono>
#include <stdexcept>
#include "Telex.h"
#include "Globals.h"

#ifndef VIETTYPE_TEST
#error "Cannot build test; not in test mode"
#endif

#ifdef _DEBUG
#define ITERATIONS 10000
#else
#define ITERATIONS 10000000
#endif

using namespace Telex;

void main1() {
    TelexConfig c;
    c.oa_uy_tone1 = true;
    TelexEngine tx(c);

    if (tx.PushChar(L'n') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'g') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'u') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'y') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'e') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'e') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'n') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'x') != TELEX_STATES::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.Commit() != TELEX_STATES::COMMITTED) {
        throw std::exception("push_char error");
    }
    std::wstring str = tx.Retrieve();
    if (str != L"nguy\x1ec5n") {
        throw std::exception("retrieve bad string");
    }
}

void main2() {
    TelexConfig c;
    c.oa_uy_tone1 = true;
    TelexEngine tx(c);

    auto t1 = std::chrono::high_resolution_clock::now();
    size_t len = 0;
    for (int i = 0; i < ITERATIONS; i++) {
        tx.Reset();
        tx.PushChar(L'N');
        tx.PushChar(L'G');
        tx.PushChar(L'U');
        tx.PushChar(L'y');
        tx.PushChar(L'E');
        tx.PushChar(L'e');
        tx.PushChar(L'n');
        tx.PushChar(L'X');
        tx.Commit();
        len += tx.Retrieve().length();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::wcout << L"total time: " << ITERATIONS << L" iters = " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;
    std::wcout << L"total len = " << len << std::endl;
}

void main3() {
    TelexConfig c;
    c.oa_uy_tone1 = true;
    TelexEngine tx(c);
    
    std::wstring x;

    tx.Reset();
    tx.PushChar(L'g');
    x = tx.Peek();
    tx.PushChar(L'i');
    x = tx.Peek();
    tx.PushChar(L'o');
    x = tx.Peek();
    tx.PushChar(L'i');
    x = tx.Peek();
    tx.PushChar(L'w');
    x = tx.Peek();
    tx.PushChar(L's');
    x = tx.Peek();
    //tx.PushChar(L'f');
    //x = tx.Peek();
    tx.Commit();
    x = tx.Retrieve();
    DBGPRINT(L"%s", x.c_str());
}

int main() {
    main3();

    return 0;
}
