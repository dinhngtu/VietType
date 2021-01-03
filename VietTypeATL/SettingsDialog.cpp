// SPDX-License-Identifier: GPL-3.0-only

#include "SettingsDialog.h"

namespace VietType {

SettingsDialog::SettingsDialog(DWORD defaultEnabled, const Telex::TelexConfig& telexConfig) {
    _data.DefaultEnabled = defaultEnabled;
    _data.TelexConfig = telexConfig;
}

_Check_return_ HRESULT SettingsDialog::ShowDialog(_Out_ INT_PTR* result) {
    if (!Globals::DllInstance) {
        DBG_DPRINT(L"%s", L"cannot obtain instance");
        return E_FAIL;
    }
    auto ret = DialogBoxParam(Globals::DllInstance, MAKEINTRESOURCE(IDD_SETTINGS), GetActiveWindow(), SettingsDialogProc, reinterpret_cast<LPARAM>(this));
    if (ret <= 0) {
        WINERROR_GLE_RETURN_HRESULT(L"%s", L"DialogBoxParam failed");
    }
    *result = ret;
    return S_OK;
}

const SettingsDialogData& SettingsDialog::GetConfig() const {
    return _data;
}

INT_PTR CALLBACK SettingsDialog::SettingsDialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
    static SettingsDialog* instance = nullptr;
    if (uMsg == WM_INITDIALOG) {
        instance = reinterpret_cast<SettingsDialog*>(lParam);
        assert(instance);

        CheckDlgButton(hwndDlg, IDC_SETTINGS_DEFAULT_ENABLED, instance->Property<IDC_SETTINGS_DEFAULT_ENABLED>() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_SETTINGS_OA_UY, instance->Property<IDC_SETTINGS_OA_UY>() ? BST_CHECKED : BST_UNCHECKED);
        CheckDlgButton(hwndDlg, IDC_SETTINGS_ACCEPT_DD, instance->Property<IDC_SETTINGS_ACCEPT_DD>() ? BST_CHECKED : BST_UNCHECKED);

        return TRUE;
    }

    switch (uMsg) {
    case WM_COMMAND:
        assert(instance);
        switch (LOWORD(wParam)) {
        case IDOK:
            instance->Property<IDC_SETTINGS_DEFAULT_ENABLED>() = IsDlgButtonChecked(hwndDlg, IDC_SETTINGS_DEFAULT_ENABLED) != BST_UNCHECKED;
            instance->Property<IDC_SETTINGS_OA_UY>() = IsDlgButtonChecked(hwndDlg, IDC_SETTINGS_OA_UY) != BST_UNCHECKED;
            instance->Property<IDC_SETTINGS_ACCEPT_DD>() = IsDlgButtonChecked(hwndDlg, IDC_SETTINGS_ACCEPT_DD) != BST_UNCHECKED;

            instance = nullptr;
            EndDialog(hwndDlg, IDOK);
            return TRUE;
        case IDCANCEL:
            instance = nullptr;
            EndDialog(hwndDlg, IDCANCEL);
            return TRUE;
        }
        break;
    }

    return FALSE;
}

template <>
bool& SettingsDialog::Property<IDC_SETTINGS_OA_UY>() {
    return _data.TelexConfig.oa_uy_tone1;
}

template <>
bool& SettingsDialog::Property<IDC_SETTINGS_ACCEPT_DD>() {
    return _data.TelexConfig.accept_separate_dd;
}

template <>
bool& SettingsDialog::Property<IDC_SETTINGS_DEFAULT_ENABLED>() {
    return _data.DefaultEnabled;
}

}
