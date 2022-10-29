// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "stdafx.h"

namespace VietType {
namespace Globals {

extern HINSTANCE DllInstance;

extern const CLSID CLSID_TextService;
extern const GUID GUID_Profile;

extern const GUID GUID_LBI_INPUTMODE;
extern const GUID GUID_PROP_INPUTSCOPE;

extern const GUID GUID_Compartment_Backconvert;
extern const GUID GUID_Compartment_NeedsSetLangid;

extern const std::wstring TextServiceDescription;
extern const std::wstring ConfigKeyName;

extern const LANGID TextServiceLangId;

} // namespace Globals
} // namespace VietType
