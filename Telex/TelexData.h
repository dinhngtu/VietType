// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "TelexInternal.h"

namespace VietType {
namespace Telex {

/// <summary>
/// transition.
/// ensure that c1 or v only shrinks by 1 character or stays a constant length with this transition.
/// </summary>
extern const generic_map_type<std::wstring, std::wstring> transitions;
// which character in _v that is responsible for ResposTransitionV; deleting this character will remove the ResposTransitionV key
extern const generic_map_type<std::wstring, int> respos;

extern const generic_map_type<std::wstring, std::wstring> transitions_w;
extern const generic_map_type<std::wstring, std::wstring> transitions_w_q;
// which character in _v that is responsible for ResposTransitionW
extern const generic_map_type<std::wstring, int> respos_w;

extern const generic_map_type<std::wstring, std::wstring> transitions_v_c2;
extern const generic_map_type<wchar_t, std::wstring> transitions_tones;

extern const generic_set_type<std::wstring> valid_c1;
extern const generic_map_type<std::wstring, VInfo> valid_v; // value: accent position
extern const generic_map_type<std::wstring, VInfo> valid_v_q; // value: accent position
extern const generic_map_type<std::wstring, VInfo> valid_v_qu; // value: accent position
extern const generic_map_type<std::wstring, VInfo> valid_v_gi; // value: accent position
extern const generic_map_type<std::wstring, bool> valid_c2;
extern const generic_map_type<std::wstring, VInfo> valid_v_oa_uy;
extern const generic_map_type<wchar_t, std::wstring> backconversions;

extern const generic_map_type<wchar_t, wchar_t> touppermap;
extern const generic_map_type<wchar_t, wchar_t> tolowermap;

}
}
