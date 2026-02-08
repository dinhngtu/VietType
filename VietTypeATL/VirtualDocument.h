// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {
namespace VirtualDocument {

_Check_return_ HRESULT GetVirtualDocumentContext(
    _In_ ITfContext* context, _In_ ITfDocumentMgr* dim, _Outptr_result_maybenull_ ITfContext** pContext);

enum class FullContextType {
    Original,
    Transitory,
    Chromium,
};

_Check_return_ HRESULT GetFullContext(
    _In_ ITfContext* context,
    _In_ TfClientId clientId,
    _COM_Outptr_ ITfContext** fullContext,
    _Out_ FullContextType* contextType);

} // namespace VirtualDocument
} // namespace VietType
