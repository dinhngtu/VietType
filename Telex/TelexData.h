// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "ConStr.h"
#include "TelexMaps.h"
#include "TelexEngine.h"

#define MAKE_MAP(K, V, n, ...)                                                                                         \
    static constexpr const ArrayMap<K, V, std::initializer_list<std::pair<K, V>>{__VA_ARGS__}.size(), false> n = {     \
        __VA_ARGS__}
#define MAKE_SET(K, n, ...)                                                                                            \
    static constexpr const ArraySet<K, std::initializer_list<K>{__VA_ARGS__}.size(), false> n = {__VA_ARGS__}
#define MAKE_SORTED_MAP(K, V, n, ...)                                                                                  \
    static constexpr const ArrayMap<K, V, std::initializer_list<std::pair<K, V>>{__VA_ARGS__}.size(), true> n = {      \
        __VA_ARGS__};                                                                                                  \
    static_assert(std::is_sorted(n.begin(), n.end(), twopair_less<K, V>))
#define MAKE_SORTED_SET(K, n, ...)                                                                                     \
    static constexpr const ArraySet<K, std::initializer_list<K>{__VA_ARGS__}.size(), true> n = {__VA_ARGS__};          \
    static_assert(std::is_sorted(n.begin(), n.end()))
#define P(a, b) std::make_pair(WConStr(a), WConStr(b))
#define P1(a, b) std::make_pair(WConStr(a), b)
#define P2(a, b) std::make_pair(a, WConStr(b))
#define VI(a, b) (VInfo{a, b})

namespace VietType {
namespace Telex {

// maps that are too short are kept as generic

MAKE_SORTED_MAP(
    WConStr,
    WConStr,
    transitions,
    P(L"aa", L"\xe2"),     //
    P(L"aua", L"\xe2u"),   // relaxed transformations
    P(L"aya", L"\xe2y"),   // relaxed transformations
    P(L"ee", L"\xea"),     //
    P(L"eue", L"\xeau"),   // relaxed transformations
    P(L"iee", L"i\xea"),   //
    P(L"ieue", L"i\xeau"), // relaxed transformations
    P(L"oio", L"\xf4i"),   // relaxed transformations
    P(L"oo", L"\xf4"),     //
    P(L"uaa", L"u\xe2"),   //
    P(L"uaya", L"u\xe2y"), // relaxed transformations
    P(L"uee", L"u\xea"),   //
    P(L"uoio", L"u\xf4i"), // relaxed transformations
    P(L"uoo", L"u\xf4"),   //
    P(L"uyee", L"uy\xea"), //
    //{ L"u\x1a1" , L"\x1b0\x1a1" }, // transition in v_c2 only
    P(L"u\x1a1i", L"\x1b0\x1a1i"), //
    P(L"u\x1a1u", L"\x1b0\x1a1u"), //
    P(L"yee", L"y\xea"),           //
    P(L"yeue", L"y\xeau"),         // relaxed transformations
    P(L"\xf4o", L"oo"),            // only for 'xoong', etc.
    P(L"\x1b0o", L"\x1b0\x1a1"),   // relaxed transformations
);
static_assert(std::all_of(transitions.begin(), transitions.end(), [](const auto& x) {
    return x.second.length() <= x.first.length();
}));

MAKE_SORTED_MAP(
    WConStr,
    int,
    respos,
    P1(L"i\xea", 1),       //
    P1(L"i\xeau", 1),      // relaxed transformations
    P1(L"uy\xea", 2),      //
    P1(L"u\xe2", 1),       //
    P1(L"u\xe2y", 1),      // relaxed transformations
    P1(L"u\xea", 1),       //
    P1(L"u\xf4", 1),       //
    P1(L"u\xf4i", 1),      // relaxed transformations
    P1(L"y\xea", 1),       //
    P1(L"y\xeau", 1),      // relaxed transformations
    P1(L"\xe2", 0),        //
    P1(L"\xe2u", 0),       // relaxed transformations
    P1(L"\xe2y", 0),       // relaxed transformations
    P1(L"\xea", 0),        //
    P1(L"\xeau", 0),       // relaxed transformations
    P1(L"\xf4", 0),        //
    P1(L"\x111", 0),       //
    P1(L"\x1b0\x1a1", 0),  // keep transition for uwow
    P1(L"\x1b0\x1a1i", 0), //
    P1(L"\x1b0\x1a1u", 0), //
);
static_assert(std::all_of(respos.begin(), respos.end(), [](const auto& x) { return x.second <= x.first.length(); }));

MAKE_SORTED_MAP(
    WConStr,
    WConStr,
    transitions_w,
    P(L"a", L"\x103"),
    P(L"o", L"\x1a1"),
    P(L"oa", L"o\x103"),
    P(L"oi", L"\x1a1i"),
    P(L"u", L"\x1b0"),
    P(L"ua", L"\x1b0\x61"),
    P(L"ui", L"\x1b0i"),
    P(L"uo", L"u\x1a1"),
    P(L"uoi", L"\x1b0\x1a1i"),
    P(L"uou", L"\x1b0\x1a1u"),
    P(L"uu", L"\x1b0u"),
    P(L"\x1b0o", L"\x1b0\x1a1"),
    // identical transitions are ignored if the last "w" is typed immediately after V without repeating (e.g. "uwow")
    P(L"\x1b0\x1a1", L"\x1b0\x1a1"), //
);
static_assert(std::all_of(transitions_w.begin(), transitions_w.end(), [](const auto& x) {
    return x.second.length() == x.first.length();
}));

MAKE_MAP(
    WConStr,
    WConStr,
    transitions_w_q,
    P(L"u", L"\x1b0"),
    P(L"ua", L"u\x103"),
    P(L"uo", L"u\x1a1"),
    P(L"uoi", L"u\x1a1i"),
    P(L"\x1b0\x1a1", L"\x1b0\x1a1"), //
);
static_assert(std::all_of(transitions_w_q.begin(), transitions_w_q.end(), [](const auto& x) {
    return x.second.length() == x.first.length();
}));

MAKE_SORTED_MAP(
    WConStr,
    int,
    respos_w,
    P1(L"o\x103", 1),      //
    P1(L"u\x103", 1),      //
    P1(L"u\x1a1", 1),      //
    P1(L"\x103", 0),       //
    P1(L"\x1a1", 0),       //
    P1(L"\x1a1i", 0),      //
    P1(L"\x1b0", 0),       //
    P1(L"\x1b0i", 0),      //
    P1(L"\x1b0\x1a1", 1),  //
    P1(L"\x1b0\x1a1i", 1), //
);
static_assert(std::all_of(respos_w.begin(), respos_w.end(), [](const auto& x) {
    return x.second <= x.first.length();
}));

MAKE_MAP(
    WConStr,
    WConStr,
    transitions_v_c2,
    P(L"u\x1a1", L"\x1b0\x1a1"), //
    P(L"\x1b0"
      "a",
      L"u\x103"),                //
    P(L"\x1b0o", L"\x1b0\x1a1"), //
);
static_assert(std::all_of(transitions_v_c2.begin(), transitions_v_c2.end(), [](const auto& x) {
    return x.second.length() == x.first.length();
}));

MAKE_MAP(WConStr, WConStr, transitions_v_c2_q, P(L"\x1b0o", L"\x1b0\x1a1"));
static_assert(std::all_of(transitions_v_c2_q.begin(), transitions_v_c2_q.end(), [](const auto& x) {
    return x.second.length() == x.first.length();
}));

MAKE_SORTED_MAP(
    wchar_t,
    WConStr,
    transitions_tones,
    P2(L'a', L"a\xe0\x1ea1\x1ea3\xe1\xe3"),               //
    P2(L'e', L"e\xe8\x1eb9\x1ebb\xe9\x1ebd"),             //
    P2(L'i', L"i\xec\x1ecb\x1ec9\xed\x129"),              //
    P2(L'o', L"o\xf2\x1ecd\x1ecf\xf3\xf5"),               //
    P2(L'u', L"u\xf9\x1ee5\x1ee7\xfa\x169"),              //
    P2(L'y', L"y\x1ef3\x1ef5\x1ef7\xfd\x1ef9"),           //
    P2(L'\xe2', L"\xe2\x1ea7\x1ead\x1ea9\x1ea5\x1eab"),   //
    P2(L'\xea', L"\xea\x1ec1\x1ec7\x1ec3\x1ebf\x1ec5"),   //
    P2(L'\xf4', L"\xf4\x1ed3\x1ed9\x1ed5\x1ed1\x1ed7"),   //
    P2(L'\x103', L"\x103\x1eb1\x1eb7\x1eb3\x1eaf\x1eb5"), //
    P2(L'\x1a1', L"\x1a1\x1edd\x1ee3\x1edf\x1edb\x1ee1"), //
    P2(L'\x1b0', L"\x1b0\x1eeb\x1ef1\x1eed\x1ee9\x1eef"), //
);
static_assert(std::all_of(transitions_tones.begin(), transitions_tones.end(), [](const auto& x) {
    return x.second.length() == transitions_tones[0].second.length();
}));

MAKE_SORTED_SET(
    WConStr,
    valid_c1,
    L"",    //
    L"b",   //
    L"c",   //
    L"ch",  //
    L"d",   //
    L"g",   //
    L"gh",  //
    L"gi",  //
    L"h",   //
    L"k",   //
    L"kh",  //
    L"l",   //
    L"m",   //
    L"n",   //
    L"ng",  //
    L"ngh", //
    L"nh",  //
    L"p",   //
    L"ph",  //
    // L"qu" ,   //
    L"q",     //
    L"r",     //
    L"s",     //
    L"t",     //
    L"th",    //
    L"tr",    //
    L"v",     //
    L"x",     //
    L"\x111", //
);

MAKE_SORTED_MAP(
    WConStr,
    VInfo,
    valid_v,
    // oa_uy must be 1 as default since 0 can only be used if there is no c2
    P1(L"a", VI(0, C2Mode::Either)),          //
    P1(L"ai", VI(0, C2Mode::NoC2)),           //
    P1(L"ao", VI(0, C2Mode::NoC2)),           //
    P1(L"au", VI(0, C2Mode::NoC2)),           //
    P1(L"ay", VI(0, C2Mode::NoC2)),           //
    P1(L"e", VI(0, C2Mode::Either)),          //
    P1(L"eo", VI(0, C2Mode::NoC2)),           //
    P1(L"i", VI(0, C2Mode::Either)),          //
    P1(L"ia", VI(0, C2Mode::NoC2)),           //
    P1(L"iu", VI(0, C2Mode::NoC2)),           //
    P1(L"i\xea", VI(1, C2Mode::MustC2)),      // iê
    P1(L"i\xeau", VI(1, C2Mode::NoC2)),       // iêu
    P1(L"o", VI(0, C2Mode::Either)),          //
    P1(L"oa", VI(1, C2Mode::Either)),         // oa_uy
    P1(L"oai", VI(1, C2Mode::NoC2)),          //
    P1(L"oao", VI(1, C2Mode::NoC2)),          // does this really exist ?
    P1(L"oay", VI(1, C2Mode::NoC2)),          //
    P1(L"oe", VI(1, C2Mode::Either)),         // oa_uy
    P1(L"oeo", VI(1, C2Mode::NoC2)),          //
    P1(L"oi", VI(0, C2Mode::NoC2)),           //
    P1(L"oo", VI(1, C2Mode::MustC2)),         // oo
    P1(L"o\x103", VI(1, C2Mode::MustC2)),     // oă
    P1(L"u", VI(0, C2Mode::Either)),          //
    P1(L"ua", VI(0, C2Mode::NoC2)),           // c2 either with qu
    P1(L"uao", VI(1, C2Mode::NoC2)),          //
    P1(L"uay", VI(1, C2Mode::NoC2)),          //
    P1(L"ui", VI(0, C2Mode::NoC2)),           //
    P1(L"uy", VI(1, C2Mode::Either)),         // oa_uy, qu different
    P1(L"uyu", VI(1, C2Mode::NoC2)),          //
    P1(L"uy\xea", VI(2, C2Mode::MustC2)),     // uyê
    P1(L"u\xe2", VI(1, C2Mode::MustC2)),      // uâ
    P1(L"u\xe2y", VI(1, C2Mode::NoC2)),       // uây
    P1(L"u\xea", VI(1, C2Mode::Either)),      // uê
    P1(L"u\xf4", VI(1, C2Mode::MustC2)),      // uô
    P1(L"u\xf4i", VI(1, C2Mode::NoC2)),       // uôi
    P1(L"u\x103", VI(1, C2Mode::MustC2)),     // uă
    P1(L"u\x1a1", VI(1, C2Mode::NoC2)),       // uơ
    P1(L"y", VI(0, C2Mode::NoC2)),            //
    P1(L"y\xea", VI(1, C2Mode::MustC2)),      // yê
    P1(L"y\xeau", VI(1, C2Mode::NoC2)),       // yêu
    P1(L"\xe2", VI(0, C2Mode::MustC2)),       // â
    P1(L"\xe2u", VI(0, C2Mode::NoC2)),        // âu
    P1(L"\xe2y", VI(0, C2Mode::NoC2)),        // ây
    P1(L"\xea", VI(0, C2Mode::Either)),       // ê
    P1(L"\xeau", VI(0, C2Mode::NoC2)),        // êu
    P1(L"\xf4", VI(0, C2Mode::Either)),       // ô
    P1(L"\xf4i", VI(0, C2Mode::NoC2)),        // ôi
    P1(L"\x103", VI(0, C2Mode::MustC2)),      // ă
                                              // "uya" cannot be accented
    P1(L"\x1a1", VI(0, C2Mode::Either)),      // ơ
    P1(L"\x1a1i", VI(0, C2Mode::NoC2)),       // ơi
    P1(L"\x1b0", VI(0, C2Mode::Either)),      // ư
    P1(L"\x1b0\x61", VI(0, C2Mode::NoC2)),    // ưa
    P1(L"\x1b0i", VI(0, C2Mode::NoC2)),       // ưi
    P1(L"\x1b0u", VI(0, C2Mode::NoC2)),       // ưu
    P1(L"\x1b0\x1a1", VI(1, C2Mode::MustC2)), // ươ
    P1(L"\x1b0\x1a1i", VI(1, C2Mode::NoC2)),  // ươi
    P1(L"\x1b0\x1a1u", VI(1, C2Mode::NoC2)),  // ươu
);
static_assert(std::all_of(valid_v.begin(), valid_v.end(), [](const auto& x) {
    return x.second.tonepos <= x.first.length();
}));

MAKE_SORTED_MAP(
    WConStr,
    VInfo,
    valid_v_q,
    P1(L"ua", VI(1, C2Mode::Either)),         // c2 either with qu
    P1(L"uai", VI(1, C2Mode::NoC2)),          //
    P1(L"uao", VI(1, C2Mode::NoC2)),          //
    P1(L"uau", VI(1, C2Mode::NoC2)),          // quạu
    P1(L"uay", VI(1, C2Mode::NoC2)),          //
    P1(L"ue", VI(1, C2Mode::Either)),         //
    P1(L"ueo", VI(1, C2Mode::NoC2)),          //
    P1(L"ui", VI(1, C2Mode::Either)),         //
    P1(L"uo", VI(1, C2Mode::NoC2)),           // quọ
    P1(L"uy", VI(1, C2Mode::Either)),         //
    P1(L"uy\xea", VI(2, C2Mode::MustC2)),     // uyê
    P1(L"u\xe2", VI(1, C2Mode::MustC2)),      // uâ
    P1(L"u\xe2y", VI(1, C2Mode::NoC2)),       // uây
    P1(L"u\xea", VI(1, C2Mode::Either)),      // uê
    P1(L"u\xeau", VI(1, C2Mode::NoC2)),       // uêu
    P1(L"u\xf4", VI(1, C2Mode::Either)),      // ô
    P1(L"u\x103", VI(1, C2Mode::MustC2)),     // uă
    P1(L"u\x1a1", VI(1, C2Mode::Either)),     // uơ
    P1(L"u\x1a1i", VI(1, C2Mode::NoC2)),      // uơi
    P1(L"\x1b0\x1a1", VI(1, C2Mode::MustC2)), // ươ
);
static_assert(std::all_of(valid_v_q.begin(), valid_v_q.end(), [](const auto& x) {
    return x.second.tonepos <= x.first.length();
}));

MAKE_SORTED_MAP(
    WConStr,
    VInfo,
    valid_v_gi,
    P1(L"", VI(-1, C2Mode::Either)),          //
    P1(L"a", VI(0, C2Mode::Either)),          //
    P1(L"ai", VI(0, C2Mode::NoC2)),           //
    P1(L"ao", VI(0, C2Mode::NoC2)),           //
    P1(L"au", VI(0, C2Mode::NoC2)),           //
    P1(L"ay", VI(0, C2Mode::NoC2)),           //
    P1(L"e", VI(0, C2Mode::Either)),          //
    P1(L"eo", VI(0, C2Mode::NoC2)),           //
    P1(L"o", VI(0, C2Mode::Either)),          //
    P1(L"oi", VI(0, C2Mode::NoC2)),           //
    P1(L"o\x103", VI(1, C2Mode::MustC2)),     // oă
    P1(L"u", VI(0, C2Mode::Either)),          //
    P1(L"ua", VI(0, C2Mode::NoC2)),           // c2 either with qu
    P1(L"ui", VI(0, C2Mode::NoC2)),           //
    P1(L"u\xf4", VI(1, C2Mode::Either)),      // ô
    P1(L"\xe2", VI(0, C2Mode::MustC2)),       // â
    P1(L"\xe2u", VI(0, C2Mode::NoC2)),        // âu
    P1(L"\xe2y", VI(0, C2Mode::NoC2)),        // ây
    P1(L"\xea", VI(0, C2Mode::Either)),       // ê
    P1(L"\xeau", VI(0, C2Mode::NoC2)),        // êu
    P1(L"\xf4", VI(0, C2Mode::Either)),       // ô
    P1(L"\xf4i", VI(0, C2Mode::NoC2)),        // ôi
    P1(L"\x103", VI(0, C2Mode::MustC2)),      // ă
    P1(L"\x1a1", VI(0, C2Mode::Either)),      // ơ
    P1(L"\x1a1i", VI(0, C2Mode::NoC2)),       // ơi
    P1(L"\x1b0", VI(0, C2Mode::Either)),      // ư
    P1(L"\x1b0\x61", VI(0, C2Mode::NoC2)),    // ưa
    P1(L"\x1b0\x1a1", VI(1, C2Mode::MustC2)), // ươ
);
static_assert(std::all_of(valid_v_gi.begin(), valid_v_gi.end(), [](const auto& x) {
    return x.second.tonepos <= static_cast<int>(x.first.length());
}));

// bool is whether tones are restricted to s/j or not
MAKE_SORTED_MAP(
    WConStr,
    bool,
    valid_c2,
    P1(L"", false),   //
    P1(L"c", true),   //
    P1(L"ch", true),  //
    P1(L"k", true),   //
    P1(L"m", false),  //
    P1(L"n", false),  //
    P1(L"ng", false), //
    P1(L"nh", false), //
    P1(L"p", true),   //
    P1(L"t", true),   //
);

MAKE_MAP(
    WConStr,
    VInfo,
    valid_v_oa_uy,
    P1(L"oa", VI(0, C2Mode::Either)), //
    P1(L"oe", VI(0, C2Mode::Either)), //
    P1(L"uy", VI(0, C2Mode::Either)), //
);
static_assert(std::all_of(valid_v_oa_uy.begin(), valid_v_oa_uy.end(), [](const auto& x) {
    return x.second.tonepos <= x.first.length();
}));

MAKE_SORTED_MAP(
    wchar_t,
    WConStr,
    backconversions,
    P2(L'\xe0', L"af"),    //
    P2(L'\xe1', L"as"),    //
    P2(L'\xe2', L"aa"),    //
    P2(L'\xe3', L"ax"),    //
    P2(L'\xe8', L"ef"),    //
    P2(L'\xe9', L"es"),    //
    P2(L'\xea', L"ee"),    //
    P2(L'\xec', L"if"),    //
    P2(L'\xed', L"is"),    //
    P2(L'\xf2', L"of"),    //
    P2(L'\xf3', L"os"),    //
    P2(L'\xf4', L"oo"),    //
    P2(L'\xf5', L"ox"),    //
    P2(L'\xf9', L"uf"),    //
    P2(L'\xfa', L"us"),    //
    P2(L'\xfd', L"ys"),    //
    P2(L'\x103', L"aw"),   //
    P2(L'\x111', L"dd"),   //
    P2(L'\x129', L"ix"),   //
    P2(L'\x169', L"ux"),   //
    P2(L'\x1a1', L"ow"),   //
    P2(L'\x1b0', L"uw"),   //
    P2(L'\x1ea1', L"aj"),  //
    P2(L'\x1ea3', L"ar"),  //
    P2(L'\x1ea5', L"aas"), //
    P2(L'\x1ea7', L"aaf"), //
    P2(L'\x1ea9', L"aar"), //
    P2(L'\x1eab', L"aax"), //
    P2(L'\x1ead', L"aaj"), //
    P2(L'\x1eaf', L"aws"), //
    P2(L'\x1eb1', L"awf"), //
    P2(L'\x1eb3', L"awr"), //
    P2(L'\x1eb5', L"awx"), //
    P2(L'\x1eb7', L"awj"), //
    P2(L'\x1eb9', L"ej"),  //
    P2(L'\x1ebb', L"er"),  //
    P2(L'\x1ebd', L"ex"),  //
    P2(L'\x1ebf', L"ees"), //
    P2(L'\x1ec1', L"eef"), //
    P2(L'\x1ec3', L"eer"), //
    P2(L'\x1ec5', L"eex"), //
    P2(L'\x1ec7', L"eej"), //
    P2(L'\x1ec9', L"ir"),  //
    P2(L'\x1ecb', L"ij"),  //
    P2(L'\x1ecd', L"oj"),  //
    P2(L'\x1ecf', L"or"),  //
    P2(L'\x1ed1', L"oos"), //
    P2(L'\x1ed3', L"oof"), //
    P2(L'\x1ed5', L"oor"), //
    P2(L'\x1ed7', L"oox"), //
    P2(L'\x1ed9', L"ooj"), //
    P2(L'\x1edb', L"ows"), //
    P2(L'\x1edd', L"owf"), //
    P2(L'\x1edf', L"owr"), //
    P2(L'\x1ee1', L"owx"), //
    P2(L'\x1ee3', L"owj"), //
    P2(L'\x1ee5', L"uj"),  //
    P2(L'\x1ee7', L"ur"),  //
    P2(L'\x1ee9', L"uws"), //
    P2(L'\x1eeb', L"uwf"), //
    P2(L'\x1eed', L"uwr"), //
    P2(L'\x1eef', L"uwx"), //
    P2(L'\x1ef1', L"uwj"), //
    P2(L'\x1ef3', L"yf"),  //
    P2(L'\x1ef5', L"yj"),  //
    P2(L'\x1ef7', L"yr"),  //
    P2(L'\x1ef9', L"yx"),  //
);

// generated from engscan (optimize=0) doubletone words
MAKE_SORTED_SET(
    WConStr,
    wlist_en,
    L"airs",     //
    L"arms",     //
    L"auras",    //
    L"axis",     //
    L"barns",    //
    L"bars",     //
    L"beefs",    //
    L"beers",    //
    L"boars",    //
    L"boors",    //
    L"bores",    //
    L"boxer",    //
    L"boxers",   //
    L"boxes",    //
    L"burns",    //
    L"bursar",   //
    L"burst",    //
    L"cars",     //
    L"chairs",   //
    L"charms",   //
    L"chars",    //
    L"cheers",   //
    L"chefs",    //
    L"chiefest", //
    L"choirs",   //
    L"chores",   //
    L"churns",   //
    L"cores",    //
    L"corns",    //
    L"corset",   //
    L"curst",    //
    L"darns",    //
    L"deers",    //
    L"defends",  //
    L"defer",    //
    L"defers",   //
    L"denser",   //
    L"deters",   //
    L"doers",    //
    L"donors",   //
    L"doors",    //
    L"genres",   //
    L"germs",    //
    L"goofs",    //
    L"gores",    //
    L"hairs",    //
    L"hangars",  //
    L"harms",    //
    L"heros",    //
    L"hers",     //
    L"honors",   //
    L"hoofs",    //
    L"horns",    //
    L"horse",    //
    L"ifs",      //
    L"irs",      //
    L"korans",   //
    L"lairs",    //
    L"leers",    //
    L"lepers",   //
    L"liars",    //
    L"loafs",    //
    L"loser",    //
    L"losers",   //
    L"major",    //
    L"majors",   //
    L"mars",     //
    L"meres",    //
    L"merest",   //
    L"meters",   //
    L"metres",   //
    L"moors",    //
    L"morns",    //
    L"morons",   //
    L"motors",   //
    L"norms",    //
    L"oafs",     //
    L"oars",     //
    L"ores",     //
    L"pairs",    //
    L"pars",     //
    L"peers",    //
    L"perjure",  //
    L"perjures", //
    L"peruse",   //
    L"pesters",  //
    L"peters",   //
    L"pores",    //
    L"purees",   //
    L"queers",   //
    L"reefs",    //
    L"refer",    //
    L"refers",   //
    L"refuse",   //
    L"roars",    //
    L"roofs",    //
    L"rosary",   //
    L"rotors",   //
    L"saris",    //
    L"sexes",    //
    L"sirs",     //
    L"soars",    //
    L"sofas",    //
    L"sores",    //
    L"sorest",   //
    L"surf",     //
    L"surfs",    //
    L"tars",     //
    L"taxis",    //
    L"tenser",   //
    L"terms",    //
    L"terse",    //
    L"terser",   //
    L"testers",  //
    L"thirst",   //
    L"thorns",   //
    L"torsi",    //
    L"torso",    //
    L"tureens",  //
    L"turf",     //
    L"turfs",    //
    L"turns",    //
    L"urns",     //
    L"veers",    //
    L"verse",    //
    L"vexes",    //
    L"virus",    //
);

// generated from dualscan mode 0 (optimize=1)
MAKE_SORTED_SET(
    WConStr,
    wlist_en_2,
    L"ask",    //
    L"bask",   //
    L"bays",   //
    L"bias",   //
    L"bins",   //
    L"boar",   //
    L"boas",   //
    L"boast",  //
    L"boats",  //
    L"books",  //
    L"booms",  //
    L"bore",   //
    L"born",   //
    L"bosom",  //
    L"bums",   //
    L"bury",   //
    L"busy",   //
    L"buys",   //
    L"cask",   //
    L"chaps",  //
    L"charm",  //
    L"chasm",  //
    L"cheeks", //
    L"cheeps", //
    L"cheer",  //
    L"choir",  //
    L"chore",  //
    L"chosen", //
    L"coast",  //
    L"coats",  //
    L"coax",   //
    L"cons",   //
    L"cooks",  //
    L"core",   //
    L"cox",    //
    L"darn",   //
    L"dawns",  //
    L"deem",   //
    L"deems",  //
    L"deeps",  //
    L"dens",   //
    L"dense",  //
    L"desk",   //
    L"dins",   //
    L"disc",   //
    L"disk",   //
    L"doer",   //
    L"does",   //
    L"donor",  //
    L"doom",   //
    L"dooms",  //
    L"door",   //
    L"dose",   //
    L"dosed",  //
    L"down",   //
    L"downs",  //
    L"dusk",   //
    L"ekes",   //
    L"gangs",  //
    L"gawks",  //
    L"gee",    //
    L"gees",   //
    L"gems",   //
    L"gene",   //
    L"genes",  //
    L"genre",  //
    L"germ",   //
    L"gets",   //
    L"ghost",  //
    L"gins",   //
    L"gist",   //
    L"goats",  //
    L"goes",   //
    L"gongs",  //
    L"goons",  //
    L"gore",   //
    L"gown",   //
    L"gowns",  //
    L"gums",   //
    L"guns",   //
    L"guys",   //
    L"hawks",  //
    L"hems",   //
    L"hens",   //
    L"her",    //
    L"hims",   //
    L"hoax",   //
    L"hoes",   //
    L"hooks",  //
    L"hoops",  //
    L"hose",   //
    L"hums",   //
    L"husk",   //
    L"keen",   //
    L"keens",  //
    L"kings",  //
    L"koran",  //
    L"lawns",  //
    L"leeks",  //
    L"liar",   //
    L"lix",    //
    L"loans",  //
    L"looks",  //
    L"loon",   //
    L"lore",   //
    L"mamas",  //
    L"maps",   //
    L"mask",   //
    L"meets",  //
    L"mere",   //
    L"metes",  //
    L"moans",  //
    L"moats",  //
    L"moons",  //
    L"more",   //
    L"morn",   //
    L"moron",  //
    L"musk",   //
    L"naps",   //
    L"nieces", //
    L"noes",   //
    L"nooks",  //
    L"norm",   //
    L"nose",   //
    L"nuns",   //
    L"oaf",    //
    L"oaks",   //
    L"oar",    //
    L"oks",    //
    L"or",     //
    L"ox",     //
    L"oxen",   //
    L"pair",   //
    L"pangs",  //
    L"pans",   //
    L"papas",  //
    L"par",    //
    L"pas",    //
    L"past",   //
    L"pasta",  //
    L"pats",   //
    L"pawn",   //
    L"pawns",  //
    L"pays",   //
    L"peeks",  //
    L"peeps",  //
    L"peer",   //
    L"penes",  //
    L"pens",   //
    L"peps",   //
    L"per",    //
    L"pest",   //
    L"pets",   //
    L"photos", //
    L"pieces", //
    L"pins",   //
    L"pis",    //
    L"pits",   //
    L"poems",  //
    L"poets",  //
    L"poops",  //
    L"poor",   //
    L"pops",   //
    L"pore",   //
    L"pose",   //
    L"post",   //
    L"pots",   //
    L"puns",   //
    L"pups",   //
    L"puree",  //
    L"pus",    //
    L"puts",   //
    L"quays",  //
    L"queens", //
    L"queer",  //
    L"quips",  //
    L"reeks",  //
    L"reuse",  //
    L"rims",   //
    L"rings",  //
    L"rips",   //
    L"risk",   //
    L"roams",  //
    L"roar",   //
    L"roast",  //
    L"roes",   //
    L"rooks",  //
    L"rooms",  //
    L"rose",   //
    L"says",   //
    L"seeks",  //
    L"seem",   //
    L"seems",  //
    L"sees",   //
    L"sics",   //
    L"sings",  //
    L"sins",   //
    L"sips",   //
    L"soaks",  //
    L"soaps",  //
    L"soar",   //
    L"soon",   //
    L"sops",   //
    L"sore",   //
    L"sos",    //
    L"sox",    //
    L"sums",   //
    L"task",   //
    L"teems",  //
    L"teens",  //
    L"tens",   //
    L"tense",  //
    L"themes", //
    L"things", //
    L"thongs", //
    L"thorn",  //
    L"those",  //
    L"tings",  //
    L"tongs",  //
    L"tons",   //
    L"tore",   //
    L"torn",   //
    L"town",   //
    L"trays",  //
    L"trees",  //
    L"treks",  //
    L"trims",  //
    L"trips",  //
    L"troops", //
    L"tureen", //
    L"tusk",   //
    L"veer",   //
    L"vips",   //
    L"xix",    //
);

// generated from dualscan mode 1 (optimize=0, autocorrect=1)
MAKE_SORTED_SET(
    WConStr,
    wlist_en_ac,
    L"ah",     //
    L"ash",    //
    L"bags",   //
    L"bash",   //
    L"begs",   //
    L"bogs",   //
    L"bugs",   //
    L"cash",   //
    L"chugs",  //
    L"cogs",   //
    L"dash",   //
    L"digs",   //
    L"dogs",   //
    L"gags",   //
    L"gash",   //
    L"gigs",   //
    L"hags",   //
    L"hah",    //
    L"hash",   //
    L"hogs",   //
    L"hugs",   //
    L"kegs",   //
    L"lags",   //
    L"lash",   //
    L"legs",   //
    L"logo",   //
    L"logs",   //
    L"lugs",   //
    L"mash",   //
    L"merge",  //
    L"mugs",   //
    L"nags",   //
    L"nah",    //
    L"pagan",  //
    L"pagans", //
    L"pegs",   //
    L"pigs",   //
    L"rags",   //
    L"rash",   //
    L"rigs",   //
    L"rugs",   //
    L"saga",   //
    L"sagas",  //
    L"sags",   //
    L"sash",   //
    L"siege",  //
    L"sieges", //
    L"signs",  //
    L"sworn",  //
    L"tags",   //
    L"thugs",  //
    L"trash",  //
    L"tugs",   //
    L"two",    //
    L"twos",   //
    L"verge",  //
);

} // namespace Telex
} // namespace VietType

#undef MAKE_MAP
#undef MAKE_SET
#undef MAKE_SORTED_MAP
#undef MAKE_SORTED_SET
#undef P
#undef P1
#undef P2
#undef VI
