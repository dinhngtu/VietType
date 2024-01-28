// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"
#include "SettingsStore.h"
#include "Telex.h"

namespace VietType {

class EngineController;
class SettingsDialog;

class EngineSettingsController : public CComObjectRootEx<CComSingleThreadModel> {
public:
    EngineSettingsController() = default;
    EngineSettingsController(const EngineSettingsController&) = delete;
    EngineSettingsController& operator=(const EngineSettingsController&) = delete;
    ~EngineSettingsController() = default;

    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(EngineSettingsController)
    BEGIN_COM_MAP(EngineSettingsController)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    _Check_return_ HRESULT
    Initialize(_In_ EngineController* ec, _In_ ITfThreadMgr* threadMgr, _In_ TfClientId clientid);
    HRESULT Uninitialize();

    HRESULT LoadTelexSettings(Telex::TelexConfig& cfg);

    void IsDefaultEnabled(_Out_ DWORD* pde) const;
    void IsBackconvertOnBackspace(_Out_ DWORD* pde) const;
    void GetPreservedKeyToggle(_Out_ TF_PRESERVEDKEY* pde) const;

private:
    EngineController* _ec = nullptr;

    // system settings

    CComPtr<RegistrySetting<DWORD>> _default_enabled;
    CComPtr<RegistrySetting<DWORD>> _backconvert_on_backspace;
    CComPtr<RegistrySetting<ULONGLONG>> _pk_toggle;

    // telex settings

    CComPtr<RegistrySetting<DWORD>> _tc_oa_uy_tone1;
    CComPtr<RegistrySetting<DWORD>> _tc_accept_dd;
    CComPtr<RegistrySetting<DWORD>> _tc_backspace_invalid;
    CComPtr<RegistrySetting<DWORD>> _tc_optimize_multilang;
    CComPtr<RegistrySetting<DWORD>> _tc_autocorrect;
};

} // namespace VietType
