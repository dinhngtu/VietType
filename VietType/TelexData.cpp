#include "TelexData.h"

namespace Telex {
    std::map<std::vector<wchar_t>, std::vector<wchar_t>> const transitions = {
        { { L'a', L'a' },{ L'\xe2' } },
    { { L'a', L'w' },{ L'\x103' } },
    { { L'e', L'e' },{ L'\xea' } },
    { { L'i', L'e', L'e' },{ L'i', L'\xea' } },
    { { L'o', L'o' },{ L'\xf4' } },
    { { L'o', L'w' },{ L'\x1a1' } },
    { { L'u', L'o', L'o' },{ L'u', L'\xf4' } },
    { { L'u', L'w' },{ L'\x1b0' } },
    { { L'u', L'o', L'w' },{ L'u', L'\x1a1' } },
    { { L'u', L'\x1a1', L'i' },{ L'\x1b0', L'\x1a1', L'i' } },
    { { L'u', L'\x1a1', L'u' },{ L'\x1b0', L'\x1a1', L'u' } },
    { { L'u', L'y', L'e', L'e' },{ L'u', L'y', L'\xea' } },
    { { L'd', L'd' },{ L'\x111' } },
    };

    std::map<std::vector<wchar_t>, std::vector<wchar_t>> const transitions_v_c2 = {
        { { L'u', L'\x1a1' },{ L'\x1b0', L'\x1a1' } },
    };

    std::map<wchar_t, std::vector<wchar_t>> const transitions_tones = {
        { L'a',{ L'a', L'\xe0', L'\x1ea1', L'\x1ea3', L'\xe1', L'\xe3' } },
    { L'\xe2',{ L'\xe2', L'\x1ea7', L'\x1ead', L'\x1ea9', L'\x1ea5', L'\x1eab' } },
    { L'\x103',{ L'\x103', L'\x1eb1', L'\x1eb7', L'\x1eb3', L'\x1eaf', L'\x1eb5' } },
    { L'e',{ L'e', L'\xe8', L'\x1eb9', L'\x1ebb', L'\xe9', L'\x1ebd' } },
    { L'\xea',{ L'\xea', L'\x1ec1', L'\x1ec7', L'\x1ec3', L'\x1ebf', L'\x1ec5' } },
    { L'i',{ L'i', L'\xec', L'\x1ecb', L'\x1ec9', L'\xed', L'\x129' } },
    { L'o',{ L'o', L'\xf2', L'\x1ecd', L'\x1ecf', L'\xf3', L'\xf5' } },
    { L'\xf4',{ L'\xf4', L'\x1ed3', L'\x1ed9', L'\x1ed5', L'\x1ed1', L'\x1ed7' } },
    { L'\x1a1',{ L'\x1a1', L'\x1edd', L'\x1ee3', L'\x1edf', L'\x1edb', L'\x1ee1' } },
    { L'u',{ L'u', L'\xf9', L'\x1ee5', L'\x1ee7', L'\xfa', L'\x169' } },
    { L'\x1b0',{ L'\x1b0', L'\x1eeb', L'\x1ef1', L'\x1eed', L'\x1ee9', L'\x1eef' } },
    { L'y',{ L'y', L'\x1ef3', L'\x1ef5', L'\x1ef7', L'\xfd', L'\x1ef9' } },
    };

    std::set<std::vector<wchar_t>> const valid_c1 = {
        {},
    { L'b' },
    { L'c' },
    { L'c', L'h' },
    { L'd' },
    { L'\x111' },
    { L'g' },
    { L'g', L'h' },
    { L'g', L'i' },
    { L'h' },
    { L'k' },
    { L'k', L'h' },
    { L'l' },
    { L'm' },
    { L'n' },
    { L'n', L'g' },
    { L'n', L'g', L'h' },
    { L'p' },
    { L'p', L'h' },
    //{ L'q', L'u' },
    { L'q' },
    { L'r' },
    { L's' },
    { L't' },
    { L't', L'h' },
    { L't', L'r' },
    { L'v' },
    { L'x' },
    };

    std::map<std::vector<wchar_t>, VINFO> const valid_v = {
        { { L'a' },{ 0, C2MODE::EITHER } },
    { { L'\x103' },{ 0, C2MODE::MUSTC2 } }, // ă
    { { L'\xe2' },{ 0, C2MODE::MUSTC2 } },  // â
    { { L'e' },{ 0, C2MODE::EITHER } },
    { { L'\xea' },{ 0, C2MODE::EITHER } }, // ê
    { { L'i' },{ 0, C2MODE::EITHER } },
    { { L'o' },{ 0, C2MODE::EITHER } },
    { { L'\xf4' },{ 0, C2MODE::EITHER } },  // ô
    { { L'\x1a1' },{ 0, C2MODE::EITHER } }, // ơ
    { { L'u' },{ 0, C2MODE::EITHER } },
    { { L'\x1b0' },{ 0, C2MODE::EITHER } }, // ư
    { { L'y' },{ 0, C2MODE::NOC2 } },

    { { L'i', L'\xea' },{ 1, C2MODE::MUSTC2 } },       // iê
    { { L'o', L'a' },{ 0, C2MODE::EITHER } },          // oa_uy
    { { L'o', L'\x103' },{ 1, C2MODE::MUSTC2 } },      // oă
    { { L'u', L'\xe2' },{ 1, C2MODE::MUSTC2 } },       // uâ
    { { L'u', L'\xea' },{ 1, C2MODE::EITHER } },       // uê
    { { L'u', L'y' },{ 0, C2MODE::EITHER } },          // oa_uy, qu different
    { { L'u', L'y', L'\xea' },{ 2, C2MODE::MUSTC2 } }, // uyê
    { { L'u', L'y', L'u' },{ 1, C2MODE::NOC2 } },
    { { L'\x1b0', L'\x1a1' },{ 1, C2MODE::MUSTC2 } }, // ươ
    { { L'y', L'\xea' },{ 1, C2MODE::MUSTC2 } },      // yê

    { { L'a', L'i' },{ 0, C2MODE::NOC2 } },
    { { L'a', L'o' },{ 0, C2MODE::NOC2 } },
    { { L'a', L'u' },{ 0, C2MODE::NOC2 } },
    { { L'a', L'y' },{ 0, C2MODE::NOC2 } },
    { { L'\xe2', L'u' },{ 0, C2MODE::NOC2 } }, // âu
    { { L'\xe2', L'y' },{ 0, C2MODE::NOC2 } }, // ây
    { { L'e', L'o' },{ 0, C2MODE::NOC2 } },
    { { L'\xea', L'u' },{ 0, C2MODE::NOC2 } }, // êu
    { { L'i', L'a' },{ 0, C2MODE::NOC2 } },
    { { L'i', L'\xea', L'u' },{ 1, C2MODE::NOC2 } }, // iêu
    { { L'i', L'u' },{ 0, C2MODE::NOC2 } },
    { { L'o', L'a', L'i' },{ 1, C2MODE::NOC2 } },
    { { L'o', L'a', L'o' },{ 1, C2MODE::NOC2 } }, // does this really exist ?
    { { L'o', L'a', L'y' },{ 1, C2MODE::NOC2 } },
    { { L'o', L'e', L'o' },{ 1, C2MODE::NOC2 } },
    { { L'o', L'i' },{ 0, C2MODE::NOC2 } },
    { { L'\xf4', L'i' },{ 0, C2MODE::NOC2 } },       // ôi
    { { L'\x1a1', L'i' },{ 0, C2MODE::NOC2 } },      // ơi
    { { L'u', L'a' },{ 0, C2MODE::NOC2 } },          // c2 either with qu
    { { L'u', L'\xe2', L'y' },{ 1, C2MODE::NOC2 } }, // uây
                                                     //{ { L'u', L'a', L'i' },1 }, // qu only
    { { L'u', L'a', L'o' },{ 1, C2MODE::NOC2 } },
    { { L'u', L'a', L'y' },{ 1, C2MODE::NOC2 } },
    // { { L'u', L'e' },1 }, // qu only
    //'ueo':
    { { L'u', L'i' },{ 0, C2MODE::NOC2 } },
    { { L'u', L'\xf4' },{ 1, C2MODE::MUSTC2 } },     // uô
    { { L'u', L'\xf4', L'i' },{ 1, C2MODE::NOC2 } }, // uôi
    { { L'u', L'\x1a1' },{ 1, C2MODE::NOC2 } },      // uơ
                                                     // 'uya' cannot be accented
    { { L'\x1b0', L'a' },{ 0, C2MODE::NOC2 } },           // ưa
    { { L'\x1b0', L'i' },{ 0, C2MODE::NOC2 } },           // ưi
    { { L'\x1b0', L'\x1a1', L'i' },{ 1, C2MODE::NOC2 } }, // ươi
    { { L'\x1b0', L'\x1a1', L'u' },{ 1, C2MODE::NOC2 } }, // ươu
    { { L'\x1b0', L'u' },{ 0, C2MODE::NOC2 } },           // ưu
    { { L'y', L'\xea', L'u' },{ 1, C2MODE::NOC2 } },      // yêu
    };

    std::map<std::vector<wchar_t>, VINFO> const valid_v_q = {
        { { L'u', L'\xe2' },{ 1, C2MODE::MUSTC2 } }, // uâ
    { { L'u', L'\xea' },{ 1, C2MODE::EITHER } }, // uê
    { { L'u', L'y' },{ 1, C2MODE::EITHER } },
    { { L'u', L'y', L'\xea' },{ 2, C2MODE::MUSTC2 } }, // uyê
                                                       //{ { L'u', L'y', L'u' },1 },
    { { L'u', L'a' },{ 0, C2MODE::EITHER } },        // c2 either with qu
    { { L'u', L'\xe2', L'y' },{ 1, C2MODE::NOC2 } }, // uây
    { { L'u', L'a', L'i' },{ 1, C2MODE::NOC2 } },
    { { L'u', L'a', L'o' },{ 1, C2MODE::NOC2 } },
    { { L'u', L'a', L'y' },{ 1, C2MODE::NOC2 } },
    { { L'u', L'e' },{ 1, C2MODE::NOC2 } },
    { { L'u', L'i' },{ 1, C2MODE::NOC2 } },
    { { L'u', L'\xf4' },{ 1, C2MODE::MUSTC2 } }, // uô
    };

    // bool is whether tones are restricted to s/j or not
    std::map<std::vector<wchar_t>, bool> const valid_c2 = {
        { {}, false },
    { { L'c' }, true },
    { { L'm' }, false },
    { { L'n' }, false },
    { { L'p' }, true },
    { { L't' }, true },

    { { L'c', L'h' }, true },
    { { L'n', L'g' }, false },
    { { L'n', L'h' }, false },
    };

    std::map<std::vector<wchar_t>, VINFO> const valid_v_oa_uy = {
        { { L'o', L'a' },{ 1, C2MODE::EITHER } },{ { L'u', L'y' },{ 1, C2MODE::EITHER } }, // oa_uy, qu different
    };
} // namespace Telex
