// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <Windows.h>

namespace VietType {
namespace TestLib {

PVOID ReadWholeFile(PCWSTR filename, _Out_ PLONGLONG size);
VOID FreeFile(PVOID file);

} // namespace TestLib
} // namespace VietType
