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

void benchmark() {
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
        tx.PushChar(L'h');
        tx.Peek();
        tx.PushChar(L'i');
        tx.Peek();
        tx.PushChar(L'E');
        tx.Peek();
        tx.PushChar(L'e');
        tx.Peek();
        tx.PushChar(L'n');
        tx.Peek();
        tx.PushChar(L'G');
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

int main() {
    benchmark();

    return 0;
}
