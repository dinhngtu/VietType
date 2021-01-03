// SPDX-License-Identifier: GPL-3.0-only

#include "TelexData.h"

namespace VietType {
namespace Telex {

const generic_map_type<wchar_t, wchar_t> touppermap = {
    { L'\xe0', L'\xc0' },
    { L'\xe1', L'\xc1' },
    { L'\xe2', L'\xc2' },
    { L'\xe3', L'\xc3' },
    { L'\xe8', L'\xc8' },
    { L'\xe9', L'\xc9' },
    { L'\xea', L'\xca' },
    { L'\xec', L'\xcc' },
    { L'\xed', L'\xcd' },
    { L'\xf2', L'\xd2' },
    { L'\xf3', L'\xd3' },
    { L'\xf4', L'\xd4' },
    { L'\xf5', L'\xd5' },
    { L'\xf9', L'\xd9' },
    { L'\xfa', L'\xda' },
    { L'\xfd', L'\xdd' },
    { L'\x103', L'\x102' },
    { L'\x111', L'\x110' },
    { L'\x129', L'\x128' },
    { L'\x169', L'\x168' },
    { L'\x1a1', L'\x1a0' },
    { L'\x1b0', L'\x1af' },
};

const generic_map_type<wchar_t, wchar_t> tolowermap = {
    { L'\xc0', L'\xe0' },
    { L'\xc1', L'\xe1' },
    { L'\xc2', L'\xe2' },
    { L'\xc3', L'\xe3' },
    { L'\xc8', L'\xe8' },
    { L'\xc9', L'\xe9' },
    { L'\xca', L'\xea' },
    { L'\xcc', L'\xec' },
    { L'\xcd', L'\xed' },
    { L'\xd2', L'\xf2' },
    { L'\xd3', L'\xf3' },
    { L'\xd4', L'\xf4' },
    { L'\xd5', L'\xf5' },
    { L'\xd9', L'\xf9' },
    { L'\xda', L'\xfa' },
    { L'\xdd', L'\xfd' },
    { L'\x102', L'\x103' },
    { L'\x110', L'\x111' },
    { L'\x128', L'\x129' },
    { L'\x168', L'\x169' },
    { L'\x1a0', L'\x1a1' },
    { L'\x1af', L'\x1b0' },
};

}
}
