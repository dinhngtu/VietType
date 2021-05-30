// SPDX-License-Identifier: GPL-3.0-only

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

struct SettingsDialogData {
    bool DefaultEnabled = false;
    Telex::TelexConfig TelexConfig;
};

class SettingsDialog {
public:
    SettingsDialog() = delete;
    SettingsDialog(const SettingsDialog&) = delete;
    SettingsDialog& operator=(const SettingsDialog&) = delete;
    ~SettingsDialog() = default;

    explicit SettingsDialog(DWORD defaultEnabled, const Telex::TelexConfig& telexConfig);

    _Check_return_ HRESULT ShowDialog(_Out_ INT_PTR* result);
    const SettingsDialogData& GetConfig() const;

private:
    static INT_PTR CALLBACK SettingsDialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    // template Property<resource ID> for typesafe configuration storage
    // define the data type of the property in propType like the examples
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

    template <>
    struct propType<IDC_SETTINGS_DEFAULT_ENABLED> {
        using type = bool;
    };
    template <>
    bool& Property<IDC_SETTINGS_DEFAULT_ENABLED>();

    template <>
    struct propType<IDC_SETTINGS_BACKSPACE_INVALID> {
        using type = bool;
    };
    template <>
    bool& Property<IDC_SETTINGS_BACKSPACE_INVALID>();

private:
    SettingsDialogData _data;
};

}
