// Copyright (c) 2018 Dinh Ngoc Tu.
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

#include "EngineSettingsController.h"
#include "EngineController.h"
#include "SettingsDialog.h"

namespace VietType {

// {B2FBD2E7-922F-4996-BE77-21085B91A8F0}
static const GUID GUID_DefaultEnabledCompartment = { 0xb2fbd2e7, 0x922f, 0x4996, { 0xbe, 0x77, 0x21, 0x8, 0x5b, 0x91, 0xa8, 0xf0 } };
// {57335895-0C34-40BA-83F7-72E90A39C222}
static const GUID GUID_TelexConfigCompartment = { 0x57335895, 0xc34, 0x40ba, { 0x83, 0xf7, 0x72, 0xe9, 0xa, 0x39, 0xc2, 0x22 } };

_Check_return_ HRESULT EngineSettingsController::Initialize(
    _In_ EngineController* ec,
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId clientid) {
    HRESULT hr;

    _ec = ec;

    hr = CreateInitialize(
        &_default_enabled,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"default_enabled",
        threadMgr,
        clientid,
        GUID_DefaultEnabledCompartment);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_default_enabled) failed");
#ifdef _DEBUG
    DWORD def;
    HRESULT hrDef = _default_enabled->GetValue(&def);
    DBG_DPRINT(L"hr = %ld, def = %ld", hrDef, def);
#endif // _DEBUG

    hr = CreateInitialize(
        &_tc_oa_uy_tone1,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"oa_uy_tone1",
        threadMgr,
        clientid,
        GUID_TelexConfigCompartment,
        false,
        [this] { return LoadSettings(); });
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_tc_oa_uy_tone1) failed");

    hr = CreateInitialize(
        &_tc_accept_dd,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"accept_dd",
        threadMgr,
        clientid,
        GUID_TelexConfigCompartment,
        false,
        [this] { return LoadSettings(); });
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_tc_accept_dd) failed");

    return S_OK;
}

HRESULT EngineSettingsController::Uninitialize() {
    HRESULT hr;

    hr = _tc_accept_dd->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->Uninitialize failed");
    _tc_accept_dd.Release();

    hr = _tc_oa_uy_tone1->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->Uninitialize failed");
    _tc_oa_uy_tone1.Release();

    hr = _default_enabled->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_default_enabled->Uninitialize failed");
    _default_enabled.Release();

    return S_OK;
}

HRESULT EngineSettingsController::LoadSettings() {
    HRESULT hr;

    DWORD default_enabled;
    hr = this->IsDefaultEnabled(&default_enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"this->IsDefaultEnabled failed");

    DWORD oa_uy_tone1 = true;
    hr = _tc_oa_uy_tone1->GetValueOrWriteback(&oa_uy_tone1, _ec->GetEngine().GetConfig().oa_uy_tone1);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->GetValueOrWriteback failed");

    DWORD accept_dd = true;
    hr = _tc_accept_dd->GetValueOrWriteback(&accept_dd, _ec->GetEngine().GetConfig().accept_separate_dd);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->GetValueOrWriteback failed");

    DBG_DPRINT(L"loading settings default=%ld oa_uy=%ld accept_dd=%ld", default_enabled, oa_uy_tone1, accept_dd);

    auto cfg = _ec->GetEngine().GetConfig();
    cfg.oa_uy_tone1 = static_cast<bool>(oa_uy_tone1);
    cfg.accept_separate_dd = static_cast<bool>(accept_dd);
    _ec->GetEngine().SetConfig(cfg);

    return S_OK;
}

HRESULT EngineSettingsController::CommitSettings(const SettingsDialog& dlg) {
    HRESULT hr;
    _ec->GetEngine().SetConfig(dlg.GetConfig().TelexConfig);

    hr = _default_enabled->SetValue(static_cast<DWORD>(dlg.GetConfig().DefaultEnabled));
    hr = _tc_oa_uy_tone1->SetValue(static_cast<DWORD>(dlg.GetConfig().TelexConfig.oa_uy_tone1));
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->SetValue failed");
    hr = _tc_accept_dd->SetValue(static_cast<DWORD>(dlg.GetConfig().TelexConfig.accept_separate_dd));
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->SetValue failed");

    return S_OK;
}

_Check_return_ HRESULT EngineSettingsController::IsDefaultEnabled(_Out_ DWORD* pde) const {
    return _default_enabled->GetValueOrWriteback(pde, 0);
}

}