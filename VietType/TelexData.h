#pragma once

#include <map>
#include <set>
#include <vector>

namespace Telex {
    enum class WORDTONES {
        Z,
        F,
        J,
        R,
        S,
        X,
    };

    extern std::map<std::vector<wchar_t>, std::vector<wchar_t>> transitions;
    extern std::map<std::vector<wchar_t>, std::vector<wchar_t>> transitions_v_c2;
    extern std::map<wchar_t, std::vector<wchar_t>> transitions_tones;
    extern std::set<std::vector<wchar_t>> valid_c1;
    extern std::map<std::vector<wchar_t>, int> valid_v; // value: accent position
    extern std::set<std::vector<wchar_t>> valid_c2;
    extern std::map<std::vector<wchar_t>, int> valid_v_oa_uy;
}
