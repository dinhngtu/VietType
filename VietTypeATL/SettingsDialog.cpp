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

#include "SettingsDialog.h"

namespace VietType {

SettingsDialog::SettingsDialog(const Telex::TelexConfig& telexConfig) {
    _data.TelexConfig = telexConfig;
}

_Check_return_ HRESULT SettingsDialog::ShowDialog(_Out_ INT_PTR* result) {
    auto ret = DialogBoxParam(Globals::DllInstance, MAKEINTRESOURCE(IDD_SETTINGS), GetActiveWindow(), SettingsDialogProc, reinterpret_cast<LPARAM>(this));
    if (ret <= 0) {
        WINERROR_RETURN_HRESULT(L"%s", L"DialogBoxParam failed");
    }
    *result = ret;
    return S_OK;
}

const Telex::TelexConfig& SettingsDialog::GetTelexConfig() const {
    return _data.TelexConfig;
}

INT_PTR CALLBACK SettingsDialog::SettingsDialogProc(_In_ HWND hwndDlg, _In_ UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam) {
    static SettingsDialog* instance = nullptr;
    if (uMsg == WM_INITDIALOG) {
        instance = reinterpret_cast<SettingsDialog*>(lParam);
        assert(instance);
        CheckDlgButton(hwndDlg, IDC_SETTINGS_OA_UY, instance->Property<IDC_SETTINGS_OA_UY>() ? BST_CHECKED : BST_UNCHECKED);
        return TRUE;
    }

    switch (uMsg) {
    case WM_COMMAND:
        assert(instance);
        switch (LOWORD(wParam)) {
        case IDOK:
            instance->Property<IDC_SETTINGS_OA_UY>() = IsDlgButtonChecked(hwndDlg, IDC_SETTINGS_OA_UY) != BST_UNCHECKED;

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

}
