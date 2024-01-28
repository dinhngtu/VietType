// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "EngineSettingsController.h"
#include "EngineController.h"

namespace VietType {

static constexpr TF_PRESERVEDKEY PK_Toggle = {VK_OEM_3, TF_MOD_ALT}; // Alt-`

_Check_return_ HRESULT EngineSettingsController::Initialize(
    _In_ EngineController* ec, _In_ ITfThreadMgr* threadMgr, _In_ TfClientId clientid) {
    _ec = ec;
    _settingsKey.Open(HKEY_CURRENT_USER, Globals::ConfigKeyName, KEY_QUERY_VALUE);

    return S_OK;
}

HRESULT EngineSettingsController::Uninitialize() {
    _settingsKey.Close();
    return S_OK;
}

HRESULT EngineSettingsController::LoadTelexSettings(Telex::TelexConfig& cfg) {
    DWORD oa_uy_tone1;
    SettingsStore::GetValueOrDefault(
        _settingsKey, L"oa_uy_tone1", &oa_uy_tone1, static_cast<DWORD>(_ec->GetEngine().GetConfig().oa_uy_tone1));
    cfg.oa_uy_tone1 = !!oa_uy_tone1;

    DWORD accept_dd;
    SettingsStore::GetValueOrDefault(
        _settingsKey, L"accept_dd", &accept_dd, static_cast<DWORD>(_ec->GetEngine().GetConfig().accept_separate_dd));
    cfg.accept_separate_dd = !!accept_dd;

    DWORD backspace_invalid;
    SettingsStore::GetValueOrDefault(
        _settingsKey,
        L"backspace_invalid",
        &backspace_invalid,
        static_cast<DWORD>(_ec->GetEngine().GetConfig().backspaced_word_stays_invalid));
    cfg.backspaced_word_stays_invalid = !!backspace_invalid;

    DWORD optimize_multilang;
    SettingsStore::GetValueOrDefault(
        _settingsKey, L"optimize_multilang", &optimize_multilang, _ec->GetEngine().GetConfig().optimize_multilang);
    cfg.optimize_multilang = optimize_multilang;

    DWORD autocorrect;
    SettingsStore::GetValueOrDefault(
        _settingsKey, L"autocorrect", &autocorrect, static_cast<DWORD>(_ec->GetEngine().GetConfig().autocorrect));
    cfg.autocorrect = !!autocorrect;

    return S_OK;
}

void EngineSettingsController::IsDefaultEnabled(_Out_ DWORD* pde) {
    SettingsStore::GetValueOrDefault<DWORD>(_settingsKey, L"default_enabled", pde, 0);
}

void EngineSettingsController::IsBackconvertOnBackspace(_Out_ DWORD* pde) {
    SettingsStore::GetValueOrDefault<DWORD>(_settingsKey, L"backconvert_on_backspace", pde, 0);
}

void EngineSettingsController::GetPreservedKeyToggle(_Out_ TF_PRESERVEDKEY* pde) {
    ULONGLONG val;
    if (_settingsKey.m_hKey != NULL && _settingsKey.QueryQWORDValue(L"pk_toggle", val) == ERROR_SUCCESS) {
        *pde = TF_PRESERVEDKEY{static_cast<UINT>(val & UINT_MAX), static_cast<UINT>((val >> 32) & UINT_MAX)};
    } else {
        *pde = PK_Toggle;
    }
}

void EngineSettingsController::IsShowingComposingAttr(_Out_ DWORD* pde) {
    SettingsStore::GetValueOrDefault<DWORD>(_settingsKey, L"show_composing_attr", pde, 1);
}

} // namespace VietType
