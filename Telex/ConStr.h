// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <string>
#include <algorithm>

namespace VietType {
namespace Telex {

// original idea from Scott Schurr's str_const
template <typename Ch>
class BasicConStr {
public:
    template <size_t N>
    constexpr BasicConStr(const Ch (&s)[N]) : _s(s), _n(N - 1) {
    }

    constexpr size_t size() const {
        return _n;
    }

    constexpr size_t length() const {
        return _n;
    }

    constexpr const Ch* c_str() const {
        return _s;
    }

    constexpr std::basic_string<Ch> str() const {
        return std::basic_string<Ch>(_s, _n);
    }

    constexpr Ch operator[](std::size_t i) const {
        return _s[i];
    }

    constexpr const Ch* begin() const {
        return _s;
    }

    constexpr const Ch* end() const {
        return _s + _n;
    }

    constexpr const Ch* cbegin() const {
        return _s;
    }

    constexpr const Ch* cend() const {
        return _s + _n;
    }

    constexpr bool operator==(const BasicConStr<Ch>& b) const {
        return std::equal(begin(), end(), b.begin(), b.end());
    }

    constexpr auto operator<=>(const BasicConStr<Ch>& b) const {
        return std::lexicographical_compare_three_way(begin(), end(), b.begin(), b.end());
    }

    constexpr bool operator==(const std::basic_string<Ch>& b) const {
        return std::equal(begin(), end(), b.begin(), b.end());
    }

    constexpr auto operator<=>(const std::basic_string<Ch>& b) const {
        return std::lexicographical_compare_three_way(begin(), end(), b.begin(), b.end());
    }

    template <size_t N>
    constexpr bool operator==(const Ch (&b)[N]) const {
        return std::equal(begin(), end(), b, b + N);
    }

    template <size_t N>
    constexpr auto operator<=>(const Ch (&b)[N]) const {
        return std::lexicographical_compare_three_way(begin(), end(), b, b + N);
    }

    constexpr bool operator==(const Ch* b) const {
        const auto slen = std::char_traits<Ch>::length(b);
        return std::equal(begin(), end(), b, b + slen);
    }

    constexpr auto operator<=>(const Ch* b) const {
        const auto slen = std::char_traits<Ch>::length(b);
        return std::lexicographical_compare_three_way(begin(), end(), b, b + slen);
    }

private:
    const Ch* const _s;
    const size_t _n;
};

using ConStr = BasicConStr<char>;
using WConStr = BasicConStr<wchar_t>;

} // namespace Telex
} // namespace VietType
