#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <algorithm>

#ifdef WIN32
#define DATA(x) (TEXT("..\\..\\data\\") TEXT(x))

#else
typedef char TCHAR;
#define TEXT(x) x
#define _tcscmp strcmp
#define wcsnlen_s wcsnlen
#define DATA(x) (TEXT("./data/") TEXT(x))

#endif
