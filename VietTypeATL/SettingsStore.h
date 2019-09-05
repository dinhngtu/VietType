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
#include "Compartment.h"
#include "SinkAdvisor.h"

namespace VietType {

namespace SettingsStore {

_Check_return_ HRESULT InitializeSink(
    _In_ IUnknown* punk,
    _In_ TfClientId clientid,
    _In_ const GUID& guidCompartment,
    _In_ CompartmentBase& compartment,
    _In_ SinkAdvisor<ITfCompartmentEventSink>& sinkAdvisor,
    _In_ ITfCompartmentEventSink* sink,
    _In_ bool global = false);
HRESULT UninitializeSink(_In_ CompartmentBase& compartment, _In_ SinkAdvisor<ITfCompartmentEventSink>& sink);

template <typename U>
struct regType {
    static LSTATUS QueryValue(_In_ CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, _Out_ U& value) = delete;
    static LSTATUS SetValue(_In_ CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, _In_ const U& value) = delete;
};

template <>
struct regType<DWORD> {
    static LSTATUS QueryValue(_In_ CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, _Out_ DWORD& value) {
        return key.QueryDWORDValue(pszValueName, value);
    }
    static LSTATUS SetValue(_In_ CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, _In_ const DWORD& value) {
        return key.SetDWORDValue(pszValueName, value);
    }
};

}

template <typename T>
class NotifiedSetting :
    public ITfCompartmentEventSink {
public:
    using callback_type = std::function<HRESULT()>;

    NotifiedSetting() = default;
    NotifiedSetting(const NotifiedSetting&) = delete;
    NotifiedSetting& operator=(const NotifiedSetting&) = delete;
    ~NotifiedSetting() = default;

    virtual _Check_return_ HRESULT GetValue(_Out_ T* val) = 0;
    virtual _Check_return_ HRESULT GetValueOrWriteback(_Out_ T* val, const T& defaultValue) = 0;
    virtual HRESULT SetValue(const T& val) = 0;

    // Inherited via ITfCompartmentEventSink
    virtual STDMETHODIMP OnChange(__RPC__in REFGUID rguid) override {
        if (rguid == _guidCompartment) {
            return _callback();
        }
        return S_OK;
    }

protected:
    GUID _guidCompartment = { 0 };
    callback_type _callback = [] { return S_OK; };
};

template <typename T>
class RegistrySetting :
    public CComObjectRootEx<CComSingleThreadModel>,
    public NotifiedSetting<T> {
public:
    DECLARE_NOT_AGGREGATABLE(RegistrySetting)
    BEGIN_COM_MAP(RegistrySetting)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via Setting
    virtual _Check_return_ HRESULT GetValue(_Out_ T* val) override {
        auto err = SettingsStore::regType<T>::QueryValue(_key, _valueName.c_str(), *val);
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"QueryValue failed");
        return S_OK;
    }
    virtual _Check_return_ HRESULT GetValueOrWriteback(_Out_ T* val, const T& defaultValue) override {
        auto err = SettingsStore::regType<T>::QueryValue(_key, _valueName.c_str(), *val);
        if (err != ERROR_SUCCESS) {
            err = SettingsStore::regType<T>::SetValue(_key, _valueName.c_str(), defaultValue);
            if (err == ERROR_SUCCESS) {
                *val = defaultValue;
            }
            return err;
        }
        return err;
    }
    virtual HRESULT SetValue(const T& val) override {
        HRESULT hr;
        LSTATUS err;
        err = SettingsStore::regType<T>::SetValue(_key, _valueName.c_str(), val);
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"SetValue failed");
        if (!_notifyCompartment.GetCompartment()) {
            return S_OK;
        }
        long oldNotify;
        hr = _notifyCompartment.GetValueOrWriteback(&oldNotify, 0);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_notifyCompartment.GetValueOrWriteback failed");
        hr = _notifyCompartment.SetValue(oldNotify + 1);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_notifyCompartment.SetValue failed");
        return S_OK;
    }

    _Check_return_ HRESULT Initialize(
        _In_ HKEY hkeyParent,
        _In_ std::wstring keyName,
        _In_ std::wstring valueName,
        _In_ IUnknown* punk,
        _In_ TfClientId clientid,
        _In_ const GUID& guidNotifyCompartment,
        _In_ bool global = false,
        _In_ callback_type callback = [] { return S_OK; }) {
        _valueName = valueName;
        _callback = callback;

        auto err = _key.Create(hkeyParent, keyName.c_str(), nullptr, 0, KEY_QUERY_VALUE | KEY_SET_VALUE);
        WINERROR_CHECK_RETURN_HRESULT(err, L"%s", L"_key.Create failed");

        if (guidNotifyCompartment == GUID_NULL) {
            return S_OK;
        } else {
            return SettingsStore::InitializeSink(punk, clientid, guidNotifyCompartment, _notifyCompartment, _notifyCompartmentEventSink, this, global);
        }
    }

    HRESULT Uninitialize() {
        if (!_notifyCompartment.GetCompartment()) {
            return S_OK;
        } else {
            return SettingsStore::UninitializeSink(_notifyCompartment, _notifyCompartmentEventSink);
        }
    }

private:
    std::wstring _valueName;
    CRegKey _key;
    Compartment<long> _notifyCompartment;
    SinkAdvisor<ITfCompartmentEventSink> _notifyCompartmentEventSink;
};

template <typename T>
class CachedCompartmentSetting :
    public CComObjectRootEx<CComSingleThreadModel>,
    public NotifiedSetting<T> {
public:
    DECLARE_NOT_AGGREGATABLE(CachedCompartmentSetting)
    BEGIN_COM_MAP(CachedCompartmentSetting)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via Setting
    virtual _Check_return_ HRESULT GetValue(_Out_ T* val) override {
        if (_cache.has_value()) {
            *val = _cache.value();
            return S_OK;
        } else {
            HRESULT hr = _dataCompartment.GetValue(val);
            if (SUCCEEDED(hr)) {
                _cache = *val;
            }
            return hr;
        }
    }
    virtual _Check_return_ HRESULT GetValueOrWriteback(_Out_ T* val, const T& defaultValue) override {
        if (_cache.has_value()) {
            *val = _cache.value();
            return S_OK;
        } else {
            HRESULT hr = _dataCompartment.GetValueOrWriteback(val, defaultValue);
            _cache = *val;
            return hr;
        }
    }
    virtual HRESULT SetValue(const T& val) override {
        HRESULT hr = _dataCompartment.SetValue(val);
        if (SUCCEEDED(hr)) {
            _cache = val;
        }
        return hr;
    }

    _Check_return_ HRESULT Initialize(
        _In_ IUnknown* punk,
        _In_ TfClientId clientid,
        _In_ const GUID& guidDataCompartment,
        _In_ bool global = false,
        _In_ callback_type callback = [] { return S_OK; }) {

        _guidCompartment = guidDataCompartment;
        _callback_chain = callback;
        _callback = [this] { return CachingCallback();  };

        return SettingsStore::InitializeSink(punk, clientid, guidDataCompartment, _dataCompartment, _dataCompartmentEventSink, this, global);
    }

    HRESULT Uninitialize() {
        return SettingsStore::UninitializeSink(_dataCompartment, _dataCompartmentEventSink);
    }

private:
    HRESULT CachingCallback() {
        T val;
        HRESULT hr = _dataCompartment.GetValue(&val);
        HRESULT_CHECK_RETURN(hr, L"%s", L"_dataCompartment.GetValue failed");
        if (hr == S_OK) {
            _cache = val;
        }
        return _callback_chain();
    }

private:
    callback_type _callback_chain = [] { return S_OK; };
    std::optional<T> _cache = std::nullopt;
    Compartment<T> _dataCompartment;
    SinkAdvisor<ITfCompartmentEventSink> _dataCompartmentEventSink;
};

}
