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
#include "Telex.h"

namespace VietType {

// procedure for adding a new setting:
// - add new setting controls to the settings dialog resource
// - modify SettingsDialog.h to add a new propType + property
// - add an implementation for the setting prop in SettingsDialog.cpp
// - handle setting load/commit in SettingsDialogProc
// - add a new backing store for the setting in EngineSettingsController.h (e.g. CComPtr<RegistrySetting<DWORD>>)
// - add code to init and uninit the backing store in EngineSettingsController
// - modify EngineSettingsController::LoadSettings/CommitSettings to load/commit settings to the engine

class SettingsDialog {
public:
    SettingsDialog() = delete;
    SettingsDialog(const SettingsDialog&) = delete;
    SettingsDialog& operator=(const SettingsDialog&) = delete;
    ~SettingsDialog() = default;

    explicit SettingsDialog(const Telex::TelexConfig& telexConfig);

    _Check_return_ HRESULT ShowDialog(_Out_ INT_PTR* result);
    const Telex::TelexConfig& GetTelexConfig() const;

private:
    struct SettingsDialogData {
        Telex::TelexConfig TelexConfig;
    };

    static INT_PTR CALLBACK SettingsDialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    template <WORD resNo>
    struct propType {
        using type = int;
    };
    template <WORD resNo>
    typename propType<resNo>::type& Property() = delete;

    template <>
    struct propType<IDC_SETTINGS_OA_UY> {
        using type = bool;
    };
    template <>
    bool& Property<IDC_SETTINGS_OA_UY>();

    template <>
    struct propType<IDC_SETTINGS_ACCEPT_DD> {
        using type = bool;
    };
    template <>
    bool& Property<IDC_SETTINGS_ACCEPT_DD>();

private:
    SettingsDialogData _data;
};

}
