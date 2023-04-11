// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <algorithm>
#include <utility>
#include <array>
#include <optional>

namespace VietType {
namespace Telex {

template <typename K, typename V>
static constexpr bool twopair_less(const std::pair<K, V>& a, const std::pair<K, V>& b) {
    return a.first < b.first;
}

template <typename K, typename V, typename KK>
static constexpr bool pair_less(const std::pair<K, V>& a, const KK& b) {
    return a.first < b;
}

template <typename K, typename V, size_t N, bool sorted = false>
struct VectorMap : public std::array<std::pair<K, V>, N> {
    using const_iterator = typename std::array<std::pair<K, V>, N>::const_iterator;

    template <typename KK>
    constexpr const_iterator find(const KK& key) const {
        if (sorted) {
            auto first = std::lower_bound(this->cbegin(), this->cend(), key, pair_less<K, V, KK>);
            if (first != this->cend() && first->first == key) {
                return first;
            } else {
                return this->cend();
            }
        } else {
            return std::find_if(this->cbegin(), this->cend(), [&](const auto& p) { return p.first == key; });
        }
    }

    template <typename KK>
    constexpr std::optional<std::pair<K, V>> find_opt(const KK& key) const {
        auto it = find(key);
        if (it != this->cend()) {
            return *it;
        } else {
            return {};
        }
    }
};

template <typename K, size_t N, bool sorted = false>
struct VectorSet : public std::array<K, N> {
    using const_iterator = typename std::array<K, N>::const_iterator;

    template <typename KK>
    constexpr const_iterator find(const KK& key) const {
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

    template <typename KK>
    constexpr std::optional<K> find_opt(const KK& key) const {
        auto it = find(key);
        if (it != this->cend()) {
            return *it;
        } else {
            return {};
        }
    }
};

template <typename K, typename V, size_t N>
using generic_map_type = VectorMap<K, V, N, false>;
template <typename K, typename V, size_t N>
using sorted_map_type = VectorMap<K, V, N, true>;
template <typename T, size_t N>
using generic_set_type = VectorSet<T, N, false>;
template <typename T, size_t N>
using sorted_set_type = VectorSet<T, N, true>;

} // namespace Telex
} // namespace VietType
