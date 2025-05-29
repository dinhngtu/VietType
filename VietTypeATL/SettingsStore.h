// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

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

template <>
struct regType<ULONGLONG> {
    static LSTATUS QueryValue(_In_ CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, _Out_ ULONGLONG& value) {
        return key.QueryQWORDValue(pszValueName, value);
    }
    static LSTATUS SetValue(_In_ CRegKey& key, _In_opt_z_ LPCTSTR pszValueName, _In_ const ULONGLONG& value) {
        return key.SetQWORDValue(pszValueName, value);
    }
};

template <typename T>
HRESULT GetValueOrDefault(CRegKey& key, LPCTSTR valueName, _Out_ T* val, const T& defaultValue) {
    if (!key.m_hKey) {
        *val = defaultValue;
        return S_OK;
    }
    auto err = SettingsStore::regType<T>::QueryValue(key, valueName, *val);
    if (err == ERROR_SUCCESS) {
        return S_OK;
    } else {
        *val = defaultValue;
        return err == ERROR_FILE_NOT_FOUND ? S_OK : HRESULT_FROM_WIN32(err);
    }
}

} // namespace SettingsStore

template <typename T>
class NotifiedSetting : public ITfCompartmentEventSink {
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
    GUID _guidCompartment = {0};
    callback_type _callback = [] { return S_OK; };
};

class CompartmentNotifier : public CComObjectRootEx<CComSingleThreadModel>, public NotifiedSetting<long> {
public:
    DECLARE_NO_REGISTRY()
    DECLARE_NOT_AGGREGATABLE(CompartmentNotifier)
    BEGIN_COM_MAP(CompartmentNotifier)
    COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via Setting
    virtual _Check_return_ HRESULT GetValue(_Out_ long* val) override {
        return _compartment.GetValue(val);
    }
    virtual _Check_return_ HRESULT GetValueOrWriteback(_Out_ long* val, const long& defaultValue) override {
        return _compartment.GetValueOrWriteback(val, defaultValue);
    }
    virtual HRESULT SetValue(const long& val) override {
        return _compartment.SetValue(val);
    }
    HRESULT Increment() {
        long val;
        HRESULT hr = GetValueOrWriteback(&val, 0);
        if (SUCCEEDED(hr)) {
            return SetValue(static_cast<unsigned long>(val) + 1);
        } else {
            return hr;
        }
    }

    _Check_return_ HRESULT Initialize(
        _In_ IUnknown* punk,
        _In_ TfClientId clientid,
        _In_ const GUID& guidCompartment,
        _In_ bool global = false,
        _In_ NotifiedSetting<long>::callback_type callback = [] { return S_OK; }) {

        _guidCompartment = guidCompartment;
        _callback = callback;

        return SettingsStore::InitializeSink(
            punk, clientid, guidCompartment, _compartment, _compartmentEventSink, this, global);
    }

    HRESULT Uninitialize() {
        return SettingsStore::UninitializeSink(_compartment, _compartmentEventSink);
    }

private:
    Compartment<long> _compartment;
    SinkAdvisor<ITfCompartmentEventSink> _compartmentEventSink;
};

template <typename T>
class CachedCompartmentSetting : public CComObjectRootEx<CComSingleThreadModel>, public NotifiedSetting<T> {
public:
    DECLARE_NO_REGISTRY()
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

    _Check_return_ HRESULT GetValueDirect(_Out_ T* val) {
        return _dataCompartment.GetValue(val);
    }

    _Check_return_ HRESULT Initialize(
        _In_ IUnknown* punk,
        _In_ TfClientId clientid,
        _In_ const GUID& guidDataCompartment,
        _In_ bool global = false,
        _In_ typename NotifiedSetting<T>::callback_type callback = [] { return S_OK; }) {

        this->_guidCompartment = guidDataCompartment;
        _callback_chain = callback;
        this->_callback = [this] { return CachingCallback(); };

        return SettingsStore::InitializeSink(
            punk, clientid, guidDataCompartment, _dataCompartment, _dataCompartmentEventSink, this, global);
    }

    HRESULT Uninitialize() {
        return SettingsStore::UninitializeSink(_dataCompartment, _dataCompartmentEventSink);
    }

private:
    HRESULT CachingCallback() {
        T val;
        HRESULT hr = _dataCompartment.GetValue(&val);
        HRESULT_CHECK_RETURN(hr, L"_dataCompartment.GetValue failed");
        if (hr == S_OK) {
            _cache = val;
        }
        return _callback_chain();
    }

private:
    typename NotifiedSetting<T>::callback_type _callback_chain = [] { return S_OK; };
    std::optional<T> _cache = std::nullopt;
    Compartment<T> _dataCompartment;
    SinkAdvisor<ITfCompartmentEventSink> _dataCompartmentEventSink;
};

} // namespace VietType
