// Copyright (c) Dinh Ngoc Tu.
// 
// This file is part of VietType.
// 
// VietType is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// VietType is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with VietType.  If not, see <https://www.gnu.org/licenses/>.

#include "stdafx.h"
#include "Globals.h"

HINSTANCE VietType::Globals::dllInstance = nullptr;

// {c0dd01a1-0deb-454b-8b42-d22ced1b4b23}
const CLSID VietType::Globals::CLSID_TextService = { 0xc0dd01a1, 0x0deb, 0x454b, { 0x8b, 0x42, 0xd2, 0x2c, 0xed, 0x1b, 0x4b, 0x23 } };

// {B31B741B-63CE-413A-9B5A-D2B69C695A78}
const GUID VietType::Globals::GUID_SettingsCompartment_Toggle = { 0xb31b741b, 0x63ce, 0x413a, { 0x9b, 0x5a, 0xd2, 0xb6, 0x9c, 0x69, 0x5a, 0x78 } };

// {8D93D10A-203B-4C5F-A122-8898EF9C56F5}
const GUID VietType::Globals::GUID_Profile = { 0x8d93d10a, 0x203b, 0x4c5f, { 0xa1, 0x22, 0x88, 0x98, 0xef, 0x9c, 0x56, 0xf5 } };

// pass this as guidItem of language bar item to make it show on the input indicator (the one next to the clock) instead of the language bar
const GUID VietType::Globals::GUID_LBI_INPUTMODE = { 0x2C77A81E, 0x41CC, 0x4178,{ 0xA3, 0xA7, 0x5F, 0x8A, 0x98, 0x75, 0x68, 0xE6 } };

const GUID VietType::Globals::GUID_PROP_INPUTSCOPE = { 0x1713dd5a, 0x68e7, 0x4a5b, { 0x9a, 0xf6, 0x59, 0x2a, 0x59, 0x5c, 0x77, 0x8d } };

const std::wstring VietType::Globals::TextServiceDescription = L"VietType";

// with MAKELANGID(LANG_JAPANESE, SUBLANG_JAPANESE_JAPAN), the TIP configuration UI works correctly
// without having to call SetLanguageProfileDisplayName
const LANGID VietType::Globals::TextServiceLangId = MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
