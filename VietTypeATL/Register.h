#pragma once

#include "Common.h"

extern "C" __declspec(dllexport) HRESULT __cdecl ActivateProfiles();
extern "C" __declspec(dllexport) HRESULT __cdecl DeactivateProfiles();
extern "C" __declspec(dllexport) HRESULT __cdecl IsProfileActivated();
