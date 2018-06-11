#pragma once

#ifdef VIETTYPE_TEST
#ifdef VIETTYPE_EXPORTS
#define TESTEXPORT __declspec(dllexport)
#else
#define TESTEXPORT __declspec(dllimport)
#endif
#else
#define TESTEXPORT
#endif
