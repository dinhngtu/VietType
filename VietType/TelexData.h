#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
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

    template<typename K, typename V>
    using genmap = std::unordered_map<K, V>;
    template<typename T>
    using genset = std::unordered_set<T>;

    /// <summary>
    /// transition.
    /// ensure that c1 or v only shrinks by 1 character or stays a constant length with this transition.
    /// </summary>
    extern genmap<std::wstring, std::wstring> const transitions;
    extern genmap<std::wstring, std::wstring> const transitions_w;
    extern genmap<std::wstring, std::wstring> const transitions_v_c2;
    extern genmap<wchar_t, std::wstring> const transitions_tones;
    extern genset<std::wstring> const valid_c1;
    extern genmap<std::wstring, VINFO> const valid_v; // value: accent position
    extern genmap<std::wstring, VINFO> const valid_v_q; // value: accent position
    extern genmap<std::wstring, VINFO> const valid_v_qu; // value: accent position
    extern genmap<std::wstring, VINFO> const valid_v_gi; // value: accent position
    extern genmap<std::wstring, bool> const valid_c2;
    extern genmap<std::wstring, VINFO> const valid_v_oa_uy;

    extern genmap<wchar_t, wchar_t> const touppermap;
    extern genmap<wchar_t, wchar_t> const tolowermap;
}
