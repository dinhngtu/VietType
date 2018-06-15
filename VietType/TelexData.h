#pragma once

#include <map>
#include <unordered_map>
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

    /// <summary>
    /// transition.
    /// ensure that c1 or v only shrinks by 1 character or stays a constant length with this transition.
    /// </summary>
    extern std::unordered_map<std::wstring, std::wstring> const transitions;
    extern std::unordered_map<std::wstring, std::wstring> const transitions_w;
    extern std::unordered_map<std::wstring, std::wstring> const transitions_v_c2;
    extern std::unordered_map<wchar_t, std::wstring> const transitions_tones;
    extern std::set<std::wstring> const valid_c1;
    extern std::unordered_map<std::wstring, VINFO> const valid_v; // value: accent position
    extern std::unordered_map<std::wstring, VINFO> const valid_v_q; // value: accent position
    extern std::unordered_map<std::wstring, VINFO> const valid_v_qu; // value: accent position
    extern std::unordered_map<std::wstring, VINFO> const valid_v_gi; // value: accent position
    extern std::unordered_map<std::wstring, bool> const valid_c2;
    extern std::unordered_map<std::wstring, VINFO> const valid_v_oa_uy;

    extern std::unordered_map<wchar_t, wchar_t> const touppermap;
    extern std::unordered_map<wchar_t, wchar_t> const tolowermap;
}
