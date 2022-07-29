// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>

namespace VietType {
namespace Telex {

enum class Tones
{
    Z,
    F,
    J,
    R,
    S,
    X,
};

enum class C2Mode
{
    Either,
    MustC2,
    NoC2,
};

struct VInfo {
    int tonepos;
    C2Mode c2mode;
};

template <typename K, typename V> using generic_map_type = std::unordered_map<K, V>;
template <typename T> using generic_set_type = std::unordered_set<T>;

} // namespace Telex
} // namespace VietType
