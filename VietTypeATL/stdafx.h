// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently,
// but are changed infrequently

#pragma once

#ifndef STRICT
#define STRICT
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

#include "targetver.h"

#define _ATL_APARTMENT_THREADED

#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS // some CString constructors will be explicit

#define ATL_NO_ASSERT_ON_DESTROY_NONEXISTENT_WINDOW

#include <atlbase.h>
#include <atlcom.h>
#include <atlctl.h>

#include <functional>
#include <tuple>
#include <memory>
#include <cassert>
#include <vector>
#include <array>
#include <algorithm>
#include <iterator>
#include <optional>
#include <string>

#include <strsafe.h>
#include <msctf.h>
#include <InputScope.h>
#include <ShlObj_core.h>
#include <shellapi.h>
