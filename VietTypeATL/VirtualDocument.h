// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {
namespace VirtualDocument {

_Check_return_ HRESULT GetVirtualDocumentContext(_In_ ITfContext* context, _COM_Outptr_ ITfContext** pContext);

}
}
