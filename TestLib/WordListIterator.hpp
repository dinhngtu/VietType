// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <cstddef>
#include <cstring>
#include <cstdint>
#include <cassert>
#include <iterator>

namespace VietType {
namespace TestLib {

class WordListIterator {
public:
    using value_type = const uint16_t*;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::input_iterator_tag;

    explicit WordListIterator(value_type words, value_type wend) : _words(words), _wend(wend) {
        assert(_words < _wend);
        UpdateWordLength();
    }

    WordListIterator& operator++() {
        if (static_cast<ptrdiff_t>(_wlen) == _wend - _words)
            _words = _wend;
        else
            _words += _wlen + 1;
        UpdateWordLength();
        return *this;
    }
    WordListIterator operator++(int) {
        WordListIterator old = *this;
        ++*this;
        return old;
    }
    constexpr bool operator==(const WordListIterator& other) const {
        return other._words == _words;
    }
    constexpr bool operator!=(const WordListIterator& other) const {
        return !(*this == other);
    }
    constexpr bool operator==(value_type other) const {
        return other == _words;
    }
    constexpr bool operator!=(value_type other) const {
        return !(*this == other);
    }
    constexpr reference operator*() {
        return _words;
    }
    constexpr value_type* operator->() {
        return &_words;
    }
    constexpr size_t wlen() const {
        return _wlen;
    }
    std::wstring to_wstring() const {
        return std::wstring(_words, _words + _wlen);
    }

private:
    void UpdateWordLength() {
        _wlen = 0;
        for (_wlen = 0; _wlen < static_cast<size_t>(_wend - _words) && _words[_wlen]; _wlen++)
            ;
    }

    value_type _words, _wend;
    size_t _wlen;
};

} // namespace TestLib
} // namespace VietType
