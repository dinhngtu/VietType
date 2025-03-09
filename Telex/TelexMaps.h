// SPDX-FileCopyrightText: Copyright (c) 2023 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <algorithm>
#include <utility>
#include <array>
#include <vector>
#include <optional>
#include <cassert>

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

#define TM_INHERIT_MEMBER(return_type, var, func)                                                                      \
    constexpr return_type func() const {                                                                               \
        return var.func();                                                                                             \
    }

template <typename K, typename V, bool sorted = false>
class ArrayMap {
public:
    using value_type = std::pair<K, V>;
    using const_reference = const value_type&;
    using const_iterator = const value_type*;
    using const_pointer = const value_type*;

    constexpr const_iterator begin() const {
        return _begin;
    }
    constexpr const_iterator cbegin() const {
        return _begin;
    }
    constexpr const_iterator end() const {
        return _begin + _size;
    }
    constexpr const_iterator cend() const {
        return _begin + _size;
    }
    constexpr size_t size() const {
        return _size;
    }
    constexpr bool empty() const {
        return _size == 0;
    }
    constexpr const_pointer data() const {
        return _begin;
    }
    constexpr const_reference operator[](size_t pos) const {
        assert(pos < _size);
        return _begin[pos];
    }

    constexpr ArrayMap() = default;
    constexpr ArrayMap(const_pointer begin, size_t size) : _begin(begin), _size(size) {
        if constexpr (sorted) {
            assert(std::is_sorted(_begin, _begin + _size, twopair_less<K, V>));
        }
    }
    constexpr ArrayMap(const ArrayMap&) = default;
    constexpr ArrayMap& operator=(const ArrayMap&) = default;
    ~ArrayMap() = default;

    template <typename KK>
    constexpr const_iterator find(const KK& key) const {
        if constexpr (sorted) {
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
            return std::nullopt;
        }
    }

private:
    const_pointer _begin = nullptr;
    size_t _size = 0;
};

template <typename K, bool sorted = false>
class ArraySet {
public:
    using value_type = K;
    using const_reference = const value_type&;
    using const_iterator = const value_type*;
    using const_pointer = const value_type*;

    constexpr const_iterator begin() const {
        return _begin;
    }
    constexpr const_iterator cbegin() const {
        return _begin;
    }
    constexpr const_iterator end() const {
        return _begin + _size;
    }
    constexpr const_iterator cend() const {
        return _begin + _size;
    }
    constexpr size_t size() const {
        return _size;
    }
    constexpr bool empty() const {
        return _size == 0;
    }
    constexpr const_pointer data() const {
        return _begin;
    }
    constexpr const_reference operator[](size_t pos) const {
        assert(pos < _size);
        return _begin[pos];
    }

    constexpr ArraySet() = default;
    constexpr ArraySet(const_pointer begin, size_t size) : _begin(begin), _size(size) {
        if constexpr (sorted) {
            assert(std::is_sorted(_begin, _begin + _size));
        }
    }
    constexpr ArraySet(const ArraySet&) = default;
    constexpr ArraySet& operator=(const ArraySet&) = default;
    ~ArraySet() = default;

    template <typename KK>
    constexpr const_iterator find(const KK& key) const {
        if constexpr (sorted) {
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
            return std::nullopt;
        }
    }

private:
    const_pointer _begin = nullptr;
    size_t _size = 0;
};

#undef TM_INHERIT_MEMBER

} // namespace Telex
} // namespace VietType
