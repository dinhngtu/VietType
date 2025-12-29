// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <iterator>
#include <cwchar>

namespace VietType {
namespace TestLib {

class WordListIterator {
public:
    using value_type = const wchar_t*;
    using difference_type = ptrdiff_t;
    using reference = value_type&;
    using pointer = value_type*;
    using iterator_category = std::input_iterator_tag;

    explicit WordListIterator(value_type words, value_type wend) : _words(words), _wend(wend) {
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
    constexpr const value_type* operator->() const {
        return &_words;
    }
    constexpr size_t wlen() const {
        return _wlen;
    }

private:
    void UpdateWordLength() {
        if (_words >= _wend) {
            _wlen = 0;
            return;
        }
        auto found = std::wmemchr(_words, L'\0', _wend - _words);
        if (found) {
            _wlen = static_cast<const wchar_t*>(found) - _words;
        } else {
            _wlen = _wend - _words;
        }
    }

    const wchar_t *_words, *_wend;
    size_t _wlen;
};

} // namespace TestLib
} // namespace VietType
