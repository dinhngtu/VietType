// SPDX-License-Identifier: GPL-3.0-only

#include "TelexData.h"

namespace VietType {
namespace Telex {

// maps that are too short are kept as generic

const sorted_map_type<const wchar_t*, std::wstring> transitions = {
    {L"aa", L"\xe2"},     //
    {L"aua", L"\xe2u"},   // relaxed transformations
    {L"aya", L"\xe2y"},   // relaxed transformations
    {L"ee", L"\xea"},     //
    {L"eue", L"\xeau"},   // relaxed transformations
    {L"iee", L"i\xea"},   //
    {L"ieue", L"i\xeau"}, // relaxed transformations
    {L"oio", L"\xf4i"},   // relaxed transformations
    {L"oo", L"\xf4"},     //
    {L"uaa", L"u\xe2"},   //
    {L"uaya", L"u\xe2y"}, // relaxed transformations
    {L"uee", L"u\xea"},   //
    {L"uoio", L"u\xf4i"}, // relaxed transformations
    {L"uoo", L"u\xf4"},   //
    //{ L"u\x1a1" , L"\x1b0\x1a1" }, // transition in v_c2 only
    {L"uyee", L"uy\xea"},         //
    {L"u\x1a1i", L"\x1b0\x1a1i"}, //
    {L"u\x1a1u", L"\x1b0\x1a1u"}, //
    {L"yee", L"y\xea"},           //
    {L"yeue", L"y\xeau"},         // relaxed transformations
    {L"\xf4o", L"oo"},            // only for 'xoong', etc.
    {L"\x1b0o", L"\x1b0\x1a1"},   // relaxed transformations
};

const sorted_map_type<const wchar_t*, int> respos = {
    {L"i\xea", 1},       //
    {L"i\xeau", 1},      // relaxed transformations
    {L"uy\xea", 2},      //
    {L"u\xe2", 1},       //
    {L"u\xe2y", 1},      // relaxed transformations
    {L"u\xea", 1},       //
    {L"u\xf4", 1},       //
    {L"u\xf4i", 1},      // relaxed transformations
    {L"y\xea", 1},       //
    {L"y\xeau", 1},      // relaxed transformations
    {L"\xe2", 0},        //
    {L"\xe2u", 0},       // relaxed transformations
    {L"\xe2y", 0},       // relaxed transformations
    {L"\xea", 0},        //
    {L"\xeau", 0},       // relaxed transformations
    {L"\xf4", 0},        //
    {L"\x111", 0},       //
    {L"\x1b0\x1a1", 0},  // keep transition for uwow
    {L"\x1b0\x1a1i", 0}, //
    {L"\x1b0\x1a1u", 0}, //
};

const sorted_map_type<const wchar_t*, std::wstring> transitions_w = {
    {L"a", L"\x103"},
    {L"o", L"\x1a1"},
    {L"oa", L"o\x103"},
    {L"oi", L"\x1a1i"},
    {L"u", L"\x1b0"},
    {L"ua", L"\x1b0\x61"},
    {L"ui", L"\x1b0i"},
    {L"uo", L"u\x1a1"},
    {L"uoi", L"\x1b0\x1a1i"},
    {L"uou", L"\x1b0\x1a1u"},
    {L"uu", L"\x1b0u"},
    {L"\x1b0o", L"\x1b0\x1a1"},
    // identical transitions are ignored if the last "w" is typed immediately after V without repeating (e.g. "uwow")
    {L"\x1b0\x1a1", L"\x1b0\x1a1"},
};

const generic_map_type<const wchar_t*, std::wstring> transitions_w_q = {
    {L"u", L"\x1b0"},
    {L"ua", L"u\x103"},
    {L"uo", L"u\x1a1"},
    {L"uoi", L"u\x1a1i"},
    {L"\x1b0\x1a1", L"\x1b0\x1a1"},
};

const sorted_map_type<const wchar_t*, int> respos_w = {
    {L"o\x103", 1},
    {L"u\x103", 1},
    {L"u\x1a1", 1},
    {L"\x103", 0},
    {L"\x1a1", 0},
    {L"\x1a1i", 0},
    {L"\x1b0", 0},
    {L"\x1b0i", 0},
    {L"\x1b0\x1a1", 1},
    {L"\x1b0\x1a1i", 1},
};

const generic_map_type<const wchar_t*, std::wstring> transitions_v_c2 = {
    {L"u\x1a1", L"\x1b0\x1a1"},
    {L"\x1b0o", L"\x1b0\x1a1"},
};

const generic_map_type<const wchar_t*, std::wstring> transitions_v_c2_q = {
    {L"\x1b0o", L"\x1b0\x1a1"},
};

const sorted_map_type<wchar_t, std::wstring> transitions_tones = {
    {L'a', L"a\xe0\x1ea1\x1ea3\xe1\xe3"},
    {L'e', L"e\xe8\x1eb9\x1ebb\xe9\x1ebd"},
    {L'i', L"i\xec\x1ecb\x1ec9\xed\x129"},
    {L'o', L"o\xf2\x1ecd\x1ecf\xf3\xf5"},
    {L'u', L"u\xf9\x1ee5\x1ee7\xfa\x169"},
    {L'y', L"y\x1ef3\x1ef5\x1ef7\xfd\x1ef9"},
    {L'\xe2', L"\xe2\x1ea7\x1ead\x1ea9\x1ea5\x1eab"},
    {L'\xea', L"\xea\x1ec1\x1ec7\x1ec3\x1ebf\x1ec5"},
    {L'\xf4', L"\xf4\x1ed3\x1ed9\x1ed5\x1ed1\x1ed7"},
    {L'\x103', L"\x103\x1eb1\x1eb7\x1eb3\x1eaf\x1eb5"},
    {L'\x1a1', L"\x1a1\x1edd\x1ee3\x1edf\x1edb\x1ee1"},
    {L'\x1b0', L"\x1b0\x1eeb\x1ef1\x1eed\x1ee9\x1eef"},
};

const sorted_set_type<const wchar_t*> valid_c1 = {
    L"",
    L"b",
    L"c",
    L"ch",
    L"d",
    L"g",
    L"gh",
    L"gi",
    L"h",
    L"k",
    L"kh",
    L"l",
    L"m",
    L"n",
    L"ng",
    L"ngh",
    L"nh",
    L"p",
    L"ph",
    // L"qu" ,
    L"q",
    L"r",
    L"s",
    L"t",
    L"th",
    L"tr",
    L"v",
    L"x",
    L"\x111",
};

const sorted_map_type<const wchar_t*, VInfo> valid_v = {
    {L"a", {0, C2Mode::Either}},
    {L"ai", {0, C2Mode::NoC2}},
    {L"ao", {0, C2Mode::NoC2}},
    {L"au", {0, C2Mode::NoC2}},
    {L"ay", {0, C2Mode::NoC2}},
    {L"e", {0, C2Mode::Either}},
    {L"eo", {0, C2Mode::NoC2}},
    {L"i", {0, C2Mode::Either}},
    {L"ia", {0, C2Mode::NoC2}},
    {L"iu", {0, C2Mode::NoC2}},
    {L"i\xea", {1, C2Mode::MustC2}}, // iê
    {L"i\xeau", {1, C2Mode::NoC2}},  // iêu
    // oa_uy must be 1 as default since 0 can only be used if there is no c2
    {L"o", {0, C2Mode::Either}},
    {L"oa", {1, C2Mode::Either}}, // oa_uy
    {L"oai", {1, C2Mode::NoC2}},
    {L"oao", {1, C2Mode::NoC2}}, // does this really exist ?
    {L"oay", {1, C2Mode::NoC2}},
    {L"oe", {1, C2Mode::Either}}, // oa_uy
    {L"oeo", {1, C2Mode::NoC2}},
    {L"oi", {0, C2Mode::NoC2}},
    {L"oo", {1, C2Mode::MustC2}},     // oo
    {L"o\x103", {1, C2Mode::MustC2}}, // oă
    {L"u", {0, C2Mode::Either}},
    {L"ua", {0, C2Mode::NoC2}}, // c2 either with qu
    {L"uao", {1, C2Mode::NoC2}},
    {L"uay", {1, C2Mode::NoC2}},
    {L"ui", {0, C2Mode::NoC2}},
    {L"uy", {1, C2Mode::Either}}, // oa_uy, qu different
    {L"uyu", {1, C2Mode::NoC2}},
    {L"uy\xea", {2, C2Mode::MustC2}}, // uyê
    {L"u\xe2", {1, C2Mode::MustC2}},  // uâ
    {L"u\xe2y", {1, C2Mode::NoC2}},   // uây
    {L"u\xea", {1, C2Mode::Either}},  // uê
    {L"u\xf4", {1, C2Mode::MustC2}},  // uô
    {L"u\xf4i", {1, C2Mode::NoC2}},   // uôi
    {L"u\x1a1", {1, C2Mode::NoC2}},   // uơ
    {L"y", {0, C2Mode::NoC2}},
    {L"y\xea", {1, C2Mode::MustC2}},      // yê
    {L"y\xeau", {1, C2Mode::NoC2}},       // yêu
    {L"\xe2", {0, C2Mode::MustC2}},       // â
    {L"\xe2u", {0, C2Mode::NoC2}},        // âu
    {L"\xe2y", {0, C2Mode::NoC2}},        // ây
    {L"\xea", {0, C2Mode::Either}},       // ê
    {L"\xeau", {0, C2Mode::NoC2}},        // êu
    {L"\xf4", {0, C2Mode::Either}},       // ô
    {L"\xf4i", {0, C2Mode::NoC2}},        // ôi
    {L"\x103", {0, C2Mode::MustC2}},      // ă
                                          // "uya" cannot be accented
    {L"\x1a1", {0, C2Mode::Either}},      // ơ
    {L"\x1a1i", {0, C2Mode::NoC2}},       // ơi
    {L"\x1b0", {0, C2Mode::Either}},      // ư
    {L"\x1b0\x61", {0, C2Mode::NoC2}},    // ưa
    {L"\x1b0i", {0, C2Mode::NoC2}},       // ưi
    {L"\x1b0u", {0, C2Mode::NoC2}},       // ưu
    {L"\x1b0\x1a1", {1, C2Mode::MustC2}}, // ươ
    {L"\x1b0\x1a1i", {1, C2Mode::NoC2}},  // ươi
    {L"\x1b0\x1a1u", {1, C2Mode::NoC2}},  // ươu
};

const sorted_map_type<const wchar_t*, VInfo> valid_v_q = {
    {L"ua", {1, C2Mode::Either}},         // c2 either with qu
    {L"uai", {1, C2Mode::NoC2}},          //
    {L"uao", {1, C2Mode::NoC2}},          //
    {L"uau", {1, C2Mode::NoC2}},          // quạu
    {L"uay", {1, C2Mode::NoC2}},          //
    {L"ue", {1, C2Mode::Either}},         //
    {L"ueo", {1, C2Mode::NoC2}},          //
    {L"ui", {1, C2Mode::Either}},         //
    {L"uo", {1, C2Mode::NoC2}},           // quọ
    {L"uy", {1, C2Mode::Either}},         //
    {L"uy\xea", {2, C2Mode::MustC2}},     // uyê
    {L"u\xe2", {1, C2Mode::MustC2}},      // uâ
    {L"u\xe2y", {1, C2Mode::NoC2}},       // uây
    {L"u\xea", {1, C2Mode::Either}},      // uê
    {L"u\xf4", {1, C2Mode::Either}},      // ô
    {L"u\x103", {1, C2Mode::MustC2}},     // uă
    {L"u\x1a1", {1, C2Mode::Either}},     // uơ
    {L"u\x1a1i", {1, C2Mode::NoC2}},      // uơi
    {L"\x1b0\x1a1", {1, C2Mode::MustC2}}, // ươ
};

const generic_map_type<const wchar_t*, VInfo> valid_v_gi = {
    {L"", {-1, C2Mode::Either}},
    {L"a", {0, C2Mode::Either}},
    {L"\x103", {0, C2Mode::MustC2}}, // ă
    {L"\xe2", {0, C2Mode::MustC2}},  // â
    {L"e", {0, C2Mode::NoC2}},
    {L"\xea", {0, C2Mode::MustC2}}, // ê
    {L"o", {0, C2Mode::Either}},
    {L"\xf4", {0, C2Mode::Either}},  // ô
    {L"\x1a1", {0, C2Mode::Either}}, // ơ
    {L"u", {0, C2Mode::Either}},
    {L"\x1b0", {0, C2Mode::Either}}, // ư

    {L"o\x103", {1, C2Mode::MustC2}},     // oă
    {L"\x1b0\x1a1", {1, C2Mode::MustC2}}, // ươ

    {L"ai", {0, C2Mode::NoC2}},
    {L"ao", {0, C2Mode::NoC2}},
    {L"au", {0, C2Mode::NoC2}},
    {L"ay", {0, C2Mode::NoC2}},
    {L"\xe2u", {0, C2Mode::NoC2}}, // âu
    {L"\xe2y", {0, C2Mode::NoC2}}, // ây
    {L"eo", {0, C2Mode::NoC2}},
    {L"\xeau", {0, C2Mode::NoC2}}, // êu
    {L"oi", {0, C2Mode::NoC2}},
    {L"\xf4i", {0, C2Mode::NoC2}},  // ôi
    {L"\x1a1i", {0, C2Mode::NoC2}}, // ơi
    {L"ua", {0, C2Mode::NoC2}},     // c2 either with qu
    {L"ui", {0, C2Mode::NoC2}},

    {L"\x1b0\x61", {0, C2Mode::NoC2}}, // ưa
};

// bool is whether tones are restricted to s/j or not
const sorted_map_type<const wchar_t*, bool> valid_c2 = {
    {L"", false},
    {L"c", true},
    {L"ch", true},
    {L"k", true},
    {L"m", false},
    {L"n", false},
    {L"ng", false},
    {L"nh", false},
    {L"p", true},
    {L"t", true},
};

const generic_map_type<const wchar_t*, VInfo> valid_v_oa_uy = {
    {L"oa", {0, C2Mode::Either}},
    {L"oe", {0, C2Mode::Either}},
    {L"uy", {0, C2Mode::Either}},
};

const sorted_map_type<wchar_t, std::wstring> backconversions = {
    {L'\xe0', L"af"},    {L'\xe1', L"as"},    {L'\xe2', L"aa"},    {L'\xe3', L"ax"},    {L'\xe8', L"ef"},
    {L'\xe9', L"es"},    {L'\xea', L"ee"},    {L'\xec', L"if"},    {L'\xed', L"is"},    {L'\xf2', L"of"},
    {L'\xf3', L"os"},    {L'\xf4', L"oo"},    {L'\xf5', L"ox"},    {L'\xf9', L"uf"},    {L'\xfa', L"us"},
    {L'\xfd', L"ys"},    {L'\x103', L"aw"},   {L'\x111', L"dd"},   {L'\x129', L"ix"},   {L'\x169', L"ux"},
    {L'\x1a1', L"ow"},   {L'\x1b0', L"uw"},   {L'\x1ea1', L"aj"},  {L'\x1ea3', L"ar"},  {L'\x1ea5', L"aas"},
    {L'\x1ea7', L"aaf"}, {L'\x1ea9', L"aar"}, {L'\x1eab', L"aax"}, {L'\x1ead', L"aaj"}, {L'\x1eaf', L"aws"},
    {L'\x1eb1', L"awf"}, {L'\x1eb3', L"awr"}, {L'\x1eb5', L"awx"}, {L'\x1eb7', L"awj"}, {L'\x1eb9', L"ej"},
    {L'\x1ebb', L"er"},  {L'\x1ebd', L"ex"},  {L'\x1ebf', L"ees"}, {L'\x1ec1', L"eef"}, {L'\x1ec3', L"eer"},
    {L'\x1ec5', L"eex"}, {L'\x1ec7', L"eej"}, {L'\x1ec9', L"ir"},  {L'\x1ecb', L"ij"},  {L'\x1ecd', L"oj"},
    {L'\x1ecf', L"or"},  {L'\x1ed1', L"oos"}, {L'\x1ed3', L"oof"}, {L'\x1ed5', L"oor"}, {L'\x1ed7', L"oox"},
    {L'\x1ed9', L"ooj"}, {L'\x1edb', L"ows"}, {L'\x1edd', L"owf"}, {L'\x1edf', L"owr"}, {L'\x1ee1', L"owx"},
    {L'\x1ee3', L"owj"}, {L'\x1ee5', L"uj"},  {L'\x1ee7', L"ur"},  {L'\x1ee9', L"uws"}, {L'\x1eeb', L"uwf"},
    {L'\x1eed', L"uwr"}, {L'\x1eef', L"uwx"}, {L'\x1ef1', L"uwj"}, {L'\x1ef3', L"yf"},  {L'\x1ef5', L"yj"},
    {L'\x1ef7', L"yr"},  {L'\x1ef9', L"yx"},
};

} // namespace Telex
} // namespace VietType
