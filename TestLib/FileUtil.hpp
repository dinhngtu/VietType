// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

namespace VietType {
namespace TestLib {

void* ReadWholeFile(const TCHAR* filename, _Out_ long long* size);
void FreeFile(void* file);

} // namespace TestLib
} // namespace VietType
