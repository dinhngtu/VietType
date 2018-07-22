// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <vector>

namespace VietType {
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
extern const genmap<std::wstring, std::wstring> transitions;
// which character in _v that is responsible for RESPOS_TRANSITION_V; deleting this character will remove the RESPOS_TRANSITION_V key
extern const genmap<std::wstring, int> respos;

extern const genmap<std::wstring, std::wstring> transitions_w;
// which character in _v that is responsible for RESPOS_TRANSITION_W
extern const genmap<std::wstring, int> respos_w;

extern const genmap<std::wstring, std::wstring> transitions_v_c2;
extern const genmap<wchar_t, std::wstring> transitions_tones;

extern const genset<std::wstring> valid_c1;
extern const genmap<std::wstring, VINFO> valid_v; // value: accent position
extern const genmap<std::wstring, VINFO> valid_v_q; // value: accent position
extern const genmap<std::wstring, VINFO> valid_v_qu; // value: accent position
extern const genmap<std::wstring, VINFO> valid_v_gi; // value: accent position
extern const genmap<std::wstring, bool> valid_c2;
extern const genmap<std::wstring, VINFO> valid_v_oa_uy;
extern const genmap<wchar_t, std::wstring> backconversions;

extern const genmap<wchar_t, wchar_t> touppermap;
extern const genmap<wchar_t, wchar_t> tolowermap;

}
}
