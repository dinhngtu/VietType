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

using namespace VietType::Telex;

void main1() {
    TelexConfig c;
    c.oa_uy_tone1 = true;
    TelexEngine tx(c);

    if (tx.PushChar(L'n') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'g') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'u') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'y') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'e') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'e') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'n') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.PushChar(L'x') != TelexStates::VALID) {
        throw std::exception("push_char error");
    }
    if (tx.Commit() != TelexStates::COMMITTED) {
        throw std::exception("push_char error");
    }
    auto r = tx.Retrieve();
    std::wstring str(r.begin(), r.end());
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
        tx.Peek();
        tx.PushChar(L'G');
        tx.Peek();
        tx.PushChar(L'U');
        tx.Peek();
        tx.PushChar(L'y');
        tx.Peek();
        tx.PushChar(L'E');
        tx.Peek();
        tx.PushChar(L'e');
        tx.Peek();
        tx.PushChar(L'n');
        tx.Peek();
        tx.PushChar(L'X');
        tx.Peek();
        tx.Commit();
        len += tx.Retrieve().size();
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::wcout << L"total time: " << ITERATIONS << L" iters = " << std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count() << "ms" << std::endl;
    std::wcout << L"total len = " << len << std::endl;
}

void main3() {
    TelexConfig c;
    c.oa_uy_tone1 = true;
    TelexEngine tx(c);
    
    tx.Reset();
    tx.PushChar(L'd');
    tx.PushChar(L'd');
    tx.PushChar(L'o');
    tx.PushChar(L'o');
    tx.PushChar(L'n');
    tx.PushChar(L'g');
    tx.PushChar(L'f');
    tx.Backspace();
    tx.Commit();
    auto x = tx.Retrieve();
    DPRINT(L"%s", x.c_str());
}

int main() {
    main3();

    return 0;
}
