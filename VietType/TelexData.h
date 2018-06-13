#pragma once

#include <map>
#include <set>
#include <vector>

namespace Telex {
    enum class TONES {
        Z,
        F,
        J,
        R,
        S,
        X,
    };

    enum class C2MODE {
        EITHER,
        MUSTC2,
        NOC2,
    };

    struct VINFO {
        int tonepos;
        C2MODE c2mode;
    };

    extern std::map<std::vector<wchar_t>, std::vector<wchar_t>> const transitions;
    extern std::map<std::vector<wchar_t>, std::vector<wchar_t>> const transitions_v_c2;
    extern std::map<wchar_t, std::vector<wchar_t>> const transitions_tones;
    extern std::set<std::vector<wchar_t>> const valid_c1;
    extern std::map<std::vector<wchar_t>, VINFO> const valid_v; // value: accent position
    extern std::map<std::vector<wchar_t>, VINFO> const valid_v_q; // value: accent position
    extern std::map<std::vector<wchar_t>, bool> const valid_c2;
    extern std::map<std::vector<wchar_t>, VINFO> const valid_v_oa_uy;
}
