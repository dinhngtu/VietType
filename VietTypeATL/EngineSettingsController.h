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

#pragma once

#include "Common.h"
#include "SettingsStore.h"

namespace VietType {

class EngineController;
class SettingsDialog;

class EngineSettingsController :
    public CComObjectRootEx<CComSingleThreadModel> {
public:

    EngineSettingsController() = default;
    EngineSettingsController(const EngineSettingsController&) = delete;
    EngineSettingsController& operator=(const EngineSettingsController&) = delete;
    ~EngineSettingsController() = default;

    DECLARE_NOT_AGGREGATABLE(EngineSettingsController)
    BEGIN_COM_MAP(EngineSettingsController)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    _Check_return_ HRESULT Initialize(
        _In_ EngineController* ec,
        _In_ ITfThreadMgr* threadMgr,
        _In_ TfClientId clientid);
    HRESULT Uninitialize();

    HRESULT LoadSettings();
    HRESULT CommitSettings(const SettingsDialog& dlg);

    _Check_return_ HRESULT IsDefaultEnabled(_Out_ DWORD* pde) const;

private:
    EngineController* _ec = nullptr;
    CComPtr<RegistrySetting<DWORD>> _default_enabled;
    CComPtr<RegistrySetting<DWORD>> _tc_oa_uy_tone1;
    CComPtr<RegistrySetting<DWORD>> _tc_accept_dd;
};

}
