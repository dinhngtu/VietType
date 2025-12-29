// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#endif

#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <system_error>
#include <string>
#include <algorithm>
#include <chrono>
#include <filesystem>
#include <cwchar>
#include <set>
#include <thread>
#include <mutex>
#include <deque>
