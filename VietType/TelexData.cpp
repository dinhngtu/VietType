#include "TelexData.h"

namespace Telex {
    std::unordered_map<std::wstring, std::wstring> const transitions = {
        { L"aa" , L"\xe2" },
    { L"ee" , L"\xea" },
    { L"iee" , L"i\xea" },
    { L"oo" , L"\xf4" },
    { L"uoo" , L"u\xf4" },
    { L"\x1b0o" , L"\x1b0\x1a1" },
    //{ L"u\x1a1" , L"\x1b0\x1a1" }, // transition in v_c2 only
    { L"u\x1a1i" , L"\x1b0\x1a1i" },
    { L"u\x1a1u" , L"\x1b0\x1a1u" },
    { L"uyee" , L"uy\xea" },
    { L"dd" , L"\x111" },
    };

    std::unordered_map<std::wstring, std::wstring> const transitions_w = {
    { L"a" , L"\x103" },
    { L"o" , L"\x1a1" },
    { L"oi" , L"\x1a1i" },
    { L"u" , L"\x1b0" },
    { L"uo" , L"u\x1a1" },
    };

    std::unordered_map<std::wstring, std::wstring> const transitions_v_c2 = {
        { L"u\x1a1", L"\x1b0\x1a1" },
    };

    std::unordered_map<wchar_t, std::wstring> const transitions_tones = {
    { L'a', L"a\xe0\x1ea1\x1ea3\xe1\xe3" },
    { L'\xe2', L"\xe2\x1ea7\x1ead\x1ea9\x1ea5\x1eab" },
    { L'\x103', L"\x103\x1eb1\x1eb7\x1eb3\x1eaf\x1eb5" },
    { L'e', L"e\xe8\x1eb9\x1ebb\xe9\x1ebd" },
    { L'\xea', L"\xea\x1ec1\x1ec7\x1ec3\x1ebf\x1ec5" },
    { L'i', L"i\xec\x1ecb\x1ec9\xed\x129" },
    { L'o', L"o\xf2\x1ecd\x1ecf\xf3\xf5" },
    { L'\xf4', L"\xf4\x1ed3\x1ed9\x1ed5\x1ed1\x1ed7" },
    { L'\x1a1', L"\x1a1\x1edd\x1ee3\x1edf\x1edb\x1ee1" },
    { L'u', L"u\xf9\x1ee5\x1ee7\xfa\x169" },
    { L'\x1b0', L"\x1b0\x1eeb\x1ef1\x1eed\x1ee9\x1eef" },
    { L'y', L"y\x1ef3\x1ef5\x1ef7\xfd\x1ef9" },
    };

    std::set<std::wstring> const valid_c1 = {
     L"",
     L"b" ,
     L"c" ,
     L"ch" ,
     L"d" ,
     L"\x111" ,
     L"g" ,
     L"gh" ,
     L"gi" ,
     L"h" ,
     L"k" ,
     L"kh" ,
     L"l" ,
     L"m" ,
     L"n" ,
     L"ng" ,
     L"ngh" ,
     L"nh" ,
     L"p" ,
     L"ph" ,
     // L"qu" ,
      L"q" ,
      L"r" ,
      L"s" ,
      L"t" ,
      L"th" ,
      L"tr" ,
      L"v" ,
      L"x" ,
    };

    std::unordered_map<std::wstring, VINFO> const valid_v = {
        {L"a" , { 0, C2MODE::EITHER } },
    {L"\x103" , { 0, C2MODE::MUSTC2 } }, // ă
    {L"\xe2" , { 0, C2MODE::MUSTC2 } },  // â
    {L"e" , { 0, C2MODE::EITHER } },
    {L"\xea" , { 0, C2MODE::EITHER } }, // ê
    {L"i" , { 0, C2MODE::EITHER } },
    {L"o" , { 0, C2MODE::EITHER } },
    {L"\xf4" , { 0, C2MODE::EITHER } },  // ô
    {L"\x1a1" , { 0, C2MODE::EITHER } }, // ơ
    {L"u" , { 0, C2MODE::EITHER } },
    {L"\x1b0" , { 0, C2MODE::EITHER } }, // ư
    {L"y" , { 0, C2MODE::NOC2 } },

    {L"i\xea" , { 1, C2MODE::MUSTC2 } },       // iê
    // oa_uy must be 1 as default since 0 can only be used if there is no c2
    {L"oa" , { 1, C2MODE::EITHER } },          // oa_uy
    {L"oe" , { 1, C2MODE::EITHER } },          // oa_uy
    {L"o\x103" , { 1, C2MODE::MUSTC2 } },      // oă
    {L"u\xe2" , { 1, C2MODE::MUSTC2 } },       // uâ
    {L"u\xea" , { 1, C2MODE::EITHER } },       // uê
    {L"uy" , { 1, C2MODE::EITHER } },          // oa_uy, qu different
    {L"uy\xea" , { 2, C2MODE::MUSTC2 } }, // uyê
    {L"uyu" , { 1, C2MODE::NOC2 } },
    {L"\x1b0\x1a1" , { 1, C2MODE::MUSTC2 } }, // ươ
    {L"y\xea" , { 1, C2MODE::MUSTC2 } },      // yê

    {L"ai" , { 0, C2MODE::NOC2 } },
    {L"ao" , { 0, C2MODE::NOC2 } },
    {L"au" , { 0, C2MODE::NOC2 } },
    {L"ay" , { 0, C2MODE::NOC2 } },
    {L"\xe2u" , { 0, C2MODE::NOC2 } }, // âu
    {L"\xe2y" , { 0, C2MODE::NOC2 } }, // ây
    {L"eo" , { 0, C2MODE::NOC2 } },
    {L"\xeau" , { 0, C2MODE::NOC2 } }, // êu
    {L"ia" , { 0, C2MODE::NOC2 } },
    {L"i\xeau" , { 1, C2MODE::NOC2 } }, // iêu
    {L"iu" , { 0, C2MODE::NOC2 } },
    {L"oai" , { 1, C2MODE::NOC2 } },
    {L"oao" , { 1, C2MODE::NOC2 } }, // does this really exist ?
    {L"oay" , { 1, C2MODE::NOC2 } },
    {L"oeo" , { 1, C2MODE::NOC2 } },
    {L"oi" , { 0, C2MODE::NOC2 } },
    {L"\xf4i" , { 0, C2MODE::NOC2 } },       // ôi
    {L"\x1a1i" , { 0, C2MODE::NOC2 } },      // ơi
    {L"ua" , { 0, C2MODE::NOC2 } },          // c2 either with qu
    {L"u\xe2y" , { 1, C2MODE::NOC2 } }, // uây
                                                     //{L"uai" },1 }, // qu only
    {L"uao" , { 1, C2MODE::NOC2 } },
    {L"uay" , { 1, C2MODE::NOC2 } },
    // {L"ue" },1 }, // qu only
    //"ueo":
    {L"ui" , { 0, C2MODE::NOC2 } },
    {L"u\xf4" , { 1, C2MODE::MUSTC2 } },     // uô
    {L"u\xf4i" , { 1, C2MODE::NOC2 } }, // uôi
    {L"u\x1a1" , { 1, C2MODE::NOC2 } },      // uơ
                                                     // "uya" cannot be accented
    {L"\x1b0a" , { 0, C2MODE::NOC2 } },           // ưa
    {L"\x1b0i" , { 0, C2MODE::NOC2 } },           // ưi
    {L"\x1b0\x1a1i" , { 1, C2MODE::NOC2 } }, // ươi
    {L"\x1b0\x1a1u" , { 1, C2MODE::NOC2 } }, // ươu
    {L"\x1b0u" , { 0, C2MODE::NOC2 } },           // ưu
    {L"y\xeau" , { 1, C2MODE::NOC2 } },      // yêu
    };

    std::unordered_map<std::wstring, VINFO> const valid_v_q = {
        {L"u\xe2" , { 1, C2MODE::MUSTC2 } }, // uâ
    {L"u\xf4" , { 1, C2MODE::EITHER } },  // ô
    {L"u\xea" , { 1, C2MODE::EITHER } }, // uê
    {L"uy" , { 1, C2MODE::EITHER } },
    {L"uy\xea" , { 2, C2MODE::MUSTC2 } }, // uyê
                                                 //{L"uyu" },1 },
    {L"ua" , { 1, C2MODE::EITHER } },        // c2 either with qu
    {L"u\xe2y" , { 1, C2MODE::NOC2 } }, // uây
    {L"uai" , { 1, C2MODE::NOC2 } },
    {L"uao" , { 1, C2MODE::NOC2 } },
    {L"uay" , { 1, C2MODE::NOC2 } },
    {L"ue" , { 1, C2MODE::NOC2 } },
    {L"ui" , { 1, C2MODE::NOC2 } },
    {L"u\xf4" , { 1, C2MODE::MUSTC2 } }, // uô
    };

    std::unordered_map<std::wstring, VINFO> const valid_v_qu = {
        {L"\xe2" , { 0, C2MODE::MUSTC2 } }, // uâ
    {L"\xf4" , { 0, C2MODE::EITHER } },  // ô
    {L"\xea" , { 0, C2MODE::EITHER } }, // uê
    {L"y" , { 0, C2MODE::EITHER } },
    {L"y\xea" , { 1, C2MODE::MUSTC2 } }, // uyê
                                                       //{L"uyu" },1 },
    {L"a" , { 0, C2MODE::EITHER } },        // c2 either with qu
    {L"\xe2y" , { 0, C2MODE::NOC2 } }, // uây
    {L"ai" , { 0, C2MODE::NOC2 } },
    {L"ao" , { 0, C2MODE::NOC2 } },
    {L"ay" , { 0, C2MODE::NOC2 } },
    {L"e" , { 0, C2MODE::NOC2 } },
    {L"i" , { 0, C2MODE::NOC2 } },
    {L"\xf4" , { 0, C2MODE::MUSTC2 } }, // uô
    };

    std::unordered_map<std::wstring, VINFO> const valid_v_gi = {
        {L"a" , { 0, C2MODE::EITHER } },
    {L"\x103" , { 0, C2MODE::MUSTC2 } }, // ă
    {L"\xe2" , { 0, C2MODE::MUSTC2 } },  // â
    {L"e" , { 0, C2MODE::NOC2 } },
    {L"\xea" , { 0, C2MODE::MUSTC2 } }, // ê
    {L"o" , { 0, C2MODE::EITHER } },
    {L"\xf4" , { 0, C2MODE::EITHER } },  // ô
    {L"u" , { 0, C2MODE::MUSTC2 } },
    {L"\x1b0" , { 0, C2MODE::EITHER } }, // ư

    {L"o\x103" , { 1, C2MODE::MUSTC2 } },      // oă
    {L"\x1b0\x1a1" , { 1, C2MODE::MUSTC2 } }, // ươ

    {L"ai" , { 0, C2MODE::NOC2 } },
    {L"ao" , { 0, C2MODE::NOC2 } },
    {L"au" , { 0, C2MODE::NOC2 } },
    {L"ay" , { 0, C2MODE::NOC2 } },
    {L"\xe2u" , { 0, C2MODE::NOC2 } }, // âu
    {L"\xe2y" , { 0, C2MODE::NOC2 } }, // ây
    {L"eo" , { 0, C2MODE::NOC2 } },
    {L"\xeau" , { 0, C2MODE::NOC2 } }, // êu
    {L"oi" , { 0, C2MODE::NOC2 } },
    {L"\xf4i" , { 0, C2MODE::NOC2 } },       // ôi
    {L"\x1a1i" , { 0, C2MODE::NOC2 } },      // ơi
    {L"ua" , { 0, C2MODE::NOC2 } },          // c2 either with qu
    {L"ui" , { 0, C2MODE::NOC2 } },

    {L"\x1b0a" , { 0, C2MODE::NOC2 } },           // ưa
    };

    // bool is whether tones are restricted to s/j or not
    std::unordered_map<std::wstring, bool> const valid_c2 = {
        { L"", false },
    { L"c", true },
    { L"m", false },
    { L"n", false },
    { L"p", true },
    { L"t", true },

    { L"ch", true },
    { L"ng", false },
    { L"nh", false },
    };

    std::unordered_map<std::wstring, VINFO> const valid_v_oa_uy = {
        { L"oa" ,{ 0, C2MODE::EITHER } },
    { L"oe",{ 0, C2MODE::EITHER } },
    { L"uy",{ 0, C2MODE::EITHER } },
    };
} // namespace Telex
