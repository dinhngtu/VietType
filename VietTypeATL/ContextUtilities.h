// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

class CompositionManager;
class EngineController;

HRESULT OnNewContext(
    _In_opt_ ITfContext* context, _In_ CompositionManager* compositionManager, _In_ EngineController* controller);

HRESULT GetLangidRequest(_In_ ITfContext* pic, _In_ TfClientId clientId, _Out_ BOOL* result);

HRESULT SetLangidRequest(_In_ ITfContext* pic, _In_ TfClientId clientId, _In_ bool needed);

} // namespace VietType
