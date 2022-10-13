// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class CompositionManager;
class EngineController;

HRESULT OnNewContext(
    _In_ ITfContext* context, _In_ CompositionManager* compositionManager, _In_ EngineController* controller);

} // namespace VietType
