// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class CompositionManager;
class EngineController;

HRESULT IsContextEmpty(_In_ ITfContext* context, _In_ TfClientId clientid, _Out_ bool* isempty);
HRESULT OnNewContext(_In_ ITfContext *context, _In_ CompositionManager* compositionManager, _In_ EngineController* controller);

}
