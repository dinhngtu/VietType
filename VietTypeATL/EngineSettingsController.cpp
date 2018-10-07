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

#include "EngineSettingsController.h"
#include "EngineController.h"
#include "SettingsDialog.h"

namespace VietType {

// {57335895-0C34-40BA-83F7-72E90A39C222}
static const GUID GUID_TelexConfigCompartment = { 0x57335895, 0xc34, 0x40ba, { 0x83, 0xf7, 0x72, 0xe9, 0xa, 0x39, 0xc2, 0x22 } };

_Check_return_ HRESULT EngineSettingsController::Initialize(
    _In_ EngineController* ec,
    _In_ ITfThreadMgr* threadMgr,
    _In_ TfClientId clientid) {
    HRESULT hr;

    _ec = ec;

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

    return S_OK;
}

HRESULT EngineSettingsController::LoadSettings() {
    HRESULT hr;

    DWORD oa_uy_tone1 = true;
    hr = _tc_oa_uy_tone1->GetValueOrWriteback(&oa_uy_tone1, _ec->GetEngine().GetConfig().oa_uy_tone1);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->GetValueOrWriteback failed");

    DWORD accept_dd = true;
    hr = _tc_accept_dd->GetValueOrWriteback(&accept_dd, _ec->GetEngine().GetConfig().accept_separate_dd);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->GetValueOrWriteback failed");

    auto cfg = _ec->GetEngine().GetConfig();
    cfg.oa_uy_tone1 = static_cast<bool>(oa_uy_tone1);
    cfg.accept_separate_dd = static_cast<bool>(accept_dd);
    _ec->GetEngine().SetConfig(cfg);

    return S_OK;
}

HRESULT EngineSettingsController::CommitSettings(const SettingsDialog& dlg) {
    HRESULT hr;
    _ec->GetEngine().SetConfig(dlg.GetTelexConfig());

    hr = _tc_oa_uy_tone1->SetValue(static_cast<DWORD>(dlg.GetTelexConfig().oa_uy_tone1));
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->SetValue failed");
    hr = _tc_accept_dd->SetValue(static_cast<DWORD>(dlg.GetTelexConfig().accept_separate_dd));
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->SetValue failed");

    return S_OK;
}

}
