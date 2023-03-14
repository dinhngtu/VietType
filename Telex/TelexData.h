// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "TelexInternal.h"

namespace VietType {
namespace Telex {

/// <summary>
/// transition.
/// ensure that c1 or v only shrinks by 1 character or stays a constant length with this transition.
/// </summary>
extern const sorted_map_type<const wchar_t*, std::wstring> transitions;
// which character in _v that is responsible for ResposTransitionV; deleting this character will remove the
// ResposTransitionV key
extern const sorted_map_type<const wchar_t*, int> respos;

extern const sorted_map_type<const wchar_t*, std::wstring> transitions_w;
extern const generic_map_type<const wchar_t*, std::wstring> transitions_w_q;
// which character in _v that is responsible for ResposTransitionW
extern const sorted_map_type<const wchar_t*, int> respos_w;

extern const generic_map_type<const wchar_t*, std::wstring> transitions_v_c2;
extern const generic_map_type<const wchar_t*, std::wstring> transitions_v_c2_q;
extern const sorted_map_type<wchar_t, const wchar_t*> transitions_tones;

extern const sorted_set_type<const wchar_t*> valid_c1;
extern const sorted_map_type<const wchar_t*, VInfo> valid_v;    // value: accent position
extern const sorted_map_type<const wchar_t*, VInfo> valid_v_q;  // value: accent position
extern const sorted_map_type<const wchar_t*, VInfo> valid_v_gi; // value: accent position
extern const sorted_map_type<const wchar_t*, bool> valid_c2;
extern const generic_map_type<const wchar_t*, VInfo> valid_v_oa_uy;
extern const sorted_map_type<wchar_t, std::wstring> backconversions;

} // namespace Telex
} // namespace VietType
