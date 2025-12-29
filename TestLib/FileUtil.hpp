// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <filesystem>
#include <cstdint>

namespace VietType {
namespace TestLib {

void* ReadWholeFile(const std::filesystem::path& filename, int64_t* size);
void FreeFile(void* file);

} // namespace TestLib
} // namespace VietType
