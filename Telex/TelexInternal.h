// SPDX-License-Identifier: GPL-3.0-only

#pragma once

/*
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
*/
#include <algorithm>
#include <utility>
#include <vector>
#include <string>
#include <cstring>
#include <cassert>

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

template <typename K, typename V>
struct twopair_less {
    using key_type = K;
    using value_type = V;
    using pair_type = std::pair<key_type, value_type>;
    static bool func(const pair_type& a, const pair_type& b) {
        return a.first < b.first;
    }
};

template <typename V>
struct twopair_less<const wchar_t*, V> {
    using key_type = const wchar_t*;
    using value_type = V;
    using pair_type = std::pair<key_type, value_type>;
    static bool func(const pair_type& a, const pair_type& b) {
        return wcscmp(a.first, b.first) < 0;
    }
};

template <typename K, typename V, bool sorted = false>
struct VectorMap : public std::vector<std::pair<K, V>> {
    using const_iterator = typename std::vector<std::pair<K, V>>::const_iterator;

    constexpr VectorMap(std::initializer_list<std::pair<K, V>> o) : std::vector<std::pair<K, V>>(o) {
        if (sorted) {
            assert(std::is_sorted(this->cbegin(), this->cend(), twopair_less<K, V>::func));
        }
    }

    template <typename KK>
    const_iterator find(const KK& key) const {
        if (sorted) {
            auto first = std::lower_bound(this->cbegin(), this->cend(), key, pair_less<KK>);
            if (first != this->cend() && first->first == key) {
                return first;
            } else {
                return this->cend();
            }
        } else {
            return std::find_if(this->cbegin(), this->cend(), [&](const auto& p) { return p.first == key; });
        }
    }

private:
    template <typename KK>
    static bool pair_less(const std::pair<K, V>& a, const KK& b) {
        return a.first < b;
    }
};

template <typename V>
struct one_less {
    static bool func(const V& a, const V& b) {
        assert(false);
        return a.first < b.first;
    }
};

template <>
struct one_less<const wchar_t*> {
    static bool func(const wchar_t* a, const wchar_t* b) {
        return wcscmp(a, b) < 0;
    }
};

template <typename K, bool sorted = false>
struct VectorSet : public std::vector<K> {
    using const_iterator = typename std::vector<K>::const_iterator;

    constexpr VectorSet(std::initializer_list<K> o) : std::vector<K>(o) {
        if (sorted) {
            assert(std::is_sorted(this->cbegin(), this->cend(), one_less<K>::func));
        }
    }

    template <typename KK>
    const_iterator find(const KK& key) const {
        if (sorted) {
            auto first = std::lower_bound(this->cbegin(), this->cend(), key);
            if (first != this->cend() && *first == key) {
                return first;
            } else {
                return this->cend();
            }
        } else {
            return std::find(this->cbegin(), this->cend(), key);
        }
    }
};

template <typename K, typename V>
using generic_map_type = VectorMap<K, V, false>;
template <typename K, typename V>
using sorted_map_type = VectorMap<K, V, true>;
template <typename T>
using generic_set_type = VectorSet<T, false>;
template <typename T>
using sorted_set_type = VectorSet<T, true>;

} // namespace Telex
} // namespace VietType
