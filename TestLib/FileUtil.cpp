// SPDX-FileCopyrightText: Copyright (c) 2024 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include <fstream>
#include <stdexcept>
#include <system_error>
#include <cstdlib>
#include "FileUtil.hpp"

namespace VietType {
namespace TestLib {

void* ReadWholeFile(const std::filesystem::path& filename, int64_t* size) {
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::system_error(errno, std::generic_category(), "std::ifstream");
    }

    auto fsize = file.tellg();
    if (fsize == -1) {
        throw std::runtime_error("failed to get file size");
    }

    if (fsize > (10LL << 32)) {
        throw std::runtime_error("file too large");
    }

    void* bytes = std::malloc(static_cast<std::size_t>(fsize));
    if (!bytes) {
        throw std::bad_alloc();
    }

    file.seekg(0, std::ios::beg);
    if (!file.read(static_cast<char*>(bytes), fsize)) {
        std::free(bytes);
        throw std::runtime_error("failed to read file");
    }

    *size = static_cast<int64_t>(fsize);
    return bytes;
}

void FreeFile(void* file) {
    std::free(file);
}

} // namespace TestLib
} // namespace VietType
