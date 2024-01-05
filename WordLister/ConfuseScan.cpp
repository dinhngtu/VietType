#include "stdafx.h"
#include "WordListIterator.hpp"
#include "FileUtil.hpp"

#pragma pack(push, 1)
struct ConfuseData {
    static constexpr uint16_t Terminal = 0x8000;
    static constexpr uint16_t Invalid = 0x4000;

    constexpr ConfuseData(uint16_t _bits) : bits(_bits) {
    }
    constexpr ConfuseData() : bits(Invalid) {
    }

    constexpr uint16_t terminal() const {
        return bits & Terminal;
    }
    constexpr uint16_t invalid() const {
        return bits & Invalid;
    }
    constexpr uint16_t offset() const {
        return bits & 0x3fff;
    }
    constexpr uint16_t char0() const {
        return bits & 0x1f;
    }
    constexpr uint16_t char1() const {
        return (bits >> 5) & 0x1f;
    }
    constexpr uint16_t char2() const {
        return (bits >> 10) & 0x1f;
    }

    static constexpr ConfuseData at(uint16_t offset) {
        return ConfuseData(offset);
    }

    static constexpr ConfuseData leaf(uint16_t char0, uint16_t char1, uint16_t char2) {
        return ConfuseData(Terminal | char0 | (char1 << 5) | (char2 << 10));
    }

    uint16_t bits;
};

struct ConfuseTable {
    ConfuseData map[26];
};
#pragma pack(pop)

struct ConfuseRawTable {
    std::wstring word;
    size_t depth;
    uint16_t key;
    size_t active;
    bool compactible;
    ConfuseData map[26];
};

bool confusescan(const wchar_t* filename) {
    LONGLONG fsize;
    auto words = static_cast<wchar_t*>(ReadWholeFile(filename, &fsize));
    auto wend = words + fsize / sizeof(wchar_t);

    wprintf(L"making raw tables\n");
    std::map<uint16_t, ConfuseRawTable*> keys;
    std::map<std::wstring, ConfuseRawTable*> tables;
    auto root = new ConfuseRawTable{std::wstring(), 0, 0, 0, false};
    keys.emplace(0, root);
    tables.emplace(std::wstring{}, root);
    uint16_t off = 1;
    unsigned long wordcount = 0;
    for (WordListIterator w(words, wend); w != wend; w++) {
        wordcount++;
        std::wstring word(*w, w.wlen());
        auto level = root;
        std::wstring stack{};
        for (size_t i = 0; i < word.length(); i++) {
            auto c = word[i];
            if (c < L'a' || c > L'z')
                throw std::runtime_error("unexpected character");
            tables.at(stack)->active++;
            stack.push_back(c);
            auto k = c - L'a';
            if (level->map[k].terminal() || level->map[k].invalid()) {
                if (i == word.length() - 1) {
                    level->map[k] = ConfuseData::leaf(k, 32, 32);
                } else {
                    if (level->map[k].terminal())
                        level->map[level->map[k].char0()] = ConfuseData();
                    level->map[k] = ConfuseData::at(off);
                    auto newEntry = new ConfuseRawTable{word, stack.size(), off, 0, i + 4 >= word.length()};
                    keys.emplace(off, newEntry);
                    tables.emplace(stack, newEntry);
                    if (++off >= (1ull << 14) - 1)
                        throw std::runtime_error("too many tables");
                }
            }
            if (i != word.length() - 1)
                level = tables.at(stack);
        }
    }
    if (off != tables.size())
        throw std::runtime_error("table count mismatch");
    wprintf(L"made %hu raw tables for %lu words\n", off, wordcount);

    wprintf(L"compacting tables\n");
    std::map<uint16_t, ConfuseRawTable*> compacted_tables;
    std::map<uint16_t, uint16_t> compact_map; // from raw key to final key
    std::deque<uint16_t> visit;
    visit.push_back(0);
    off = 0;
    while (visit.size()) {
        auto cur = visit.front();
        visit.pop_front();
        auto t = keys.at(cur);
        for (auto& dt : t->map) {
            if (dt.terminal() || dt.invalid())
                continue;
            auto ct = keys.at(dt.offset());
            if (ct->active < 2 && ct->compactible) {
                auto word = ct->word.substr(ct->depth);
                if (word.empty() || word.length() > 3)
                    throw std::runtime_error("word too long");
                dt = ConfuseData::leaf(
                    word[0] - L'a',                          //
                    word.length() > 1 ? word[1] - L'a' : 31, //
                    word.length() > 2 ? word[2] - L'a' : 31  //
                );
            } else {
                visit.push_back(dt.offset());
            }
        }
        compacted_tables[off] = t;
        compact_map[cur] = off;
        off++;
    }
    if (off != compacted_tables.size())
        throw std::runtime_error("table count mismatch");
    wprintf(L"made %hu compacted tables\n", off);

    wprintf(L"fixing up table keys\n");
    for (auto& t : compacted_tables)
        for (auto& dt : t.second->map)
            if (!dt.terminal() && !dt.invalid())
                dt = ConfuseData::at(compact_map.at(dt.offset()));

    wprintf(L"allocating tables\n");
    std::vector<ConfuseTable> ptables(compacted_tables.size());
    for (auto& t : compacted_tables)
        std::copy(std::begin(t.second->map), std::end(t.second->map), std::begin(ptables.at(t.first).map));

    wprintf(L"checking table references\n");
    for (auto& t : ptables)
        for (auto& dt : t.map)
            if (!dt.terminal() && !dt.invalid())
                if (dt.offset() >= ptables.size())
                    throw std::runtime_error("invalid table offset");
    wprintf(L"table references ok\n");

    wprintf(L"total size %zu bytes\n", ptables.size() * sizeof(ptables[0]));

    free(words);
    return true;
}
