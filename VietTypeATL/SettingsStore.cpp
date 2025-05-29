// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#include "SettingsStore.h"

namespace VietType {
namespace SettingsStore {

_Check_return_ HRESULT InitializeSink(
    _In_ IUnknown* punk,
    _In_ TfClientId clientid,
    _In_ const GUID& guidCompartment,
    _In_ CompartmentBase& compartment,
    _In_ SinkAdvisor<ITfCompartmentEventSink>& sinkAdvisor,
    _In_ ITfCompartmentEventSink* sink,
    _In_ bool global) {
    HRESULT hr = compartment.Initialize(punk, clientid, guidCompartment, global);
    HRESULT_CHECK_RETURN(hr, L"compartment.Initialize failed");

    CComPtr<ITfSource> source;
    hr = compartment.GetCompartmentSource(&source);
    HRESULT_CHECK_RETURN(hr, L"_notifyCompartment.GetCompartmentSource failed");
    hr = sinkAdvisor.Advise(source, sink);
    HRESULT_CHECK_RETURN(hr, L"_compartmentEventSink.Advise failed");

    return S_OK;
}

HRESULT UninitializeSink(_In_ CompartmentBase& compartment, _In_ SinkAdvisor<ITfCompartmentEventSink>& sinkAdvisor) {
    HRESULT hr = sinkAdvisor.Unadvise();
    HRESULT_CHECK_RETURN(hr, L"_compartmentEventSink.Unadvise failed");
    hr = compartment.Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"_notifyCompartment.Uninitialize failed");
    return S_OK;
}

} // namespace SettingsStore
} // namespace VietType
