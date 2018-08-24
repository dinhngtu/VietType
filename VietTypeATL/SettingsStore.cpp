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
    HRESULT_CHECK_RETURN(hr, L"%s", L"compartment.Initialize failed");

    CComPtr<ITfSource> source;
    hr = compartment.GetCompartmentSource(&source);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_notifyCompartment.GetCompartmentSource failed");
    hr = sinkAdvisor.Advise(source, sink);
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compartmentEventSink.Advise failed");

    return S_OK;
}

HRESULT UninitializeSink(_In_ CompartmentBase& compartment, _In_ SinkAdvisor<ITfCompartmentEventSink>& sinkAdvisor) {
    HRESULT hr = sinkAdvisor.Unadvise();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_compartmentEventSink.Unadvise failed");
    hr = compartment.Uninitialize();
    HRESULT_CHECK_RETURN(hr, L"%s", L"_notifyCompartment.Uninitialize failed");
    return S_OK;
}

}
}
