// Copyright (c) 2018 Dinh Ngoc Tu.
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

#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>

namespace VietType {
namespace Telex {

enum class Tones {
    Z,
    F,
    J,
    R,
    S,
    X,
};

enum class C2Mode {
    Either,
    MustC2,
    NoC2,
};

struct VInfo {
    int tonepos;
    C2Mode c2mode;
};

template<typename K, typename V>
using generic_map_type = std::unordered_map<K, V>;
template<typename T>
using generic_set_type = std::unordered_set<T>;

}
}
