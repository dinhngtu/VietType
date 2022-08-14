// SPDX-License-Identifier: GPL-3.0-only

#include "EngineSettingsController.h"
#include "EngineController.h"

namespace VietType {

// {B2FBD2E7-922F-4996-BE77-21085B91A8F0}
static const GUID GUID_DefaultEnabledCompartment = {
    0xb2fbd2e7, 0x922f, 0x4996, {0xbe, 0x77, 0x21, 0x8, 0x5b, 0x91, 0xa8, 0xf0}};
// {57335895-0C34-40BA-83F7-72E90A39C222}
static const GUID GUID_TelexConfigCompartment = {
    0x57335895, 0xc34, 0x40ba, {0x83, 0xf7, 0x72, 0xe9, 0xa, 0x39, 0xc2, 0x22}};

_Check_return_ HRESULT EngineSettingsController::Initialize(
    _In_ EngineController* ec, _In_ ITfThreadMgr* threadMgr, _In_ TfClientId clientid) {
    HRESULT hr;

    _ec = ec;

    hr = CreateInitialize(
        &_default_enabled,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"default_enabled",
        KEY_QUERY_VALUE,
        threadMgr,
        clientid,
        GUID_DefaultEnabledCompartment);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_default_enabled) failed");

    hr = CreateInitialize(
        &_backconvert_on_backspace,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"backconvert_on_backspace",
        KEY_QUERY_VALUE,
        threadMgr,
        clientid,
        GUID_DefaultEnabledCompartment);
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_backconvert_on_backspace) failed");

    hr = CreateInitialize(
        &_tc_oa_uy_tone1,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"oa_uy_tone1",
        KEY_QUERY_VALUE,
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
        KEY_QUERY_VALUE,
        threadMgr,
        clientid,
        GUID_TelexConfigCompartment,
        false,
        [this] { return LoadSettings(); });
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_tc_accept_dd) failed");

    hr = CreateInitialize(
        &_tc_backspace_invalid,
        HKEY_CURRENT_USER,
        Globals::ConfigKeyName.c_str(),
        L"backspace_invalid",
        KEY_QUERY_VALUE,
        threadMgr,
        clientid,
        GUID_TelexConfigCompartment,
        false,
        [this] { return LoadSettings(); });
    HRESULT_CHECK_RETURN(hr, L"%s", L"CreateInitialize(_tc_backspace_invalid) failed");

    return S_OK;
}

HRESULT EngineSettingsController::Uninitialize() {
    HRESULT hr;

    hr = _tc_backspace_invalid->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_backspace_invalid->Uninitialize failed");
    _tc_backspace_invalid.Release();

    hr = _tc_accept_dd->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->Uninitialize failed");
    _tc_accept_dd.Release();

    hr = _tc_oa_uy_tone1->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->Uninitialize failed");
    _tc_oa_uy_tone1.Release();

    hr = _backconvert_on_backspace->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_backconvert_on_backspace->Uninitialize failed");
    _backconvert_on_backspace.Release();

    hr = _default_enabled->Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_default_enabled->Uninitialize failed");
    _default_enabled.Release();

    return S_OK;
}

HRESULT EngineSettingsController::LoadSettings() {
    HRESULT hr;
    auto cfg(_ec->GetEngine().GetConfig());

    DWORD default_enabled;
    hr = this->IsDefaultEnabled(&default_enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"this->IsDefaultEnabled failed");

    DWORD backconvert_enabled;
    hr = this->IsBackconvertOnBackspace(&backconvert_enabled);
    HRESULT_CHECK_RETURN(hr, L"%s", L"this->IsBackconvertEnabled failed");

    DWORD oa_uy_tone1 = true;
    hr = _tc_oa_uy_tone1->GetValueOrDefault(&oa_uy_tone1, _ec->GetEngine().GetConfig().oa_uy_tone1);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_oa_uy_tone1->GetValueOrWriteback failed");
    cfg.oa_uy_tone1 = static_cast<bool>(oa_uy_tone1);

    DWORD accept_dd = true;
    hr = _tc_accept_dd->GetValueOrDefault(&accept_dd, _ec->GetEngine().GetConfig().accept_separate_dd);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_accept_dd->GetValueOrWriteback failed");
    cfg.accept_separate_dd = static_cast<bool>(accept_dd);

    DWORD backspace_invalid = true;
    hr = _tc_backspace_invalid->GetValueOrDefault(
        &backspace_invalid, _ec->GetEngine().GetConfig().backspaced_word_stays_invalid);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_tc_backspace_invalid->GetValueOrWriteback failed");
    cfg.backspaced_word_stays_invalid = static_cast<bool>(backspace_invalid);

    _ec->GetEngine().SetConfig(cfg);

    return S_OK;
}

_Check_return_ HRESULT EngineSettingsController::IsDefaultEnabled(_Out_ DWORD* pde) const {
    return _default_enabled->GetValueOrDefault(pde, 0);
}

_Check_return_ HRESULT EngineSettingsController::IsBackconvertOnBackspace(_Out_ DWORD* pde) const {
    return _backconvert_on_backspace->GetValueOrDefault(pde, 0);
}

} // namespace VietType
