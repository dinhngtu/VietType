// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {
class Context;

namespace VirtualDocument {

enum class FullContextType {
    Original,
    Transitory,
    Chromium,
};

_Check_return_ HRESULT GetFullContext(
    _In_ Context* context,
    _In_ TfClientId clientId,
    _COM_Outptr_ ITfContext** fullContext,
    _Out_ FullContextType* contextType);

} // namespace VirtualDocument
} // namespace VietType
