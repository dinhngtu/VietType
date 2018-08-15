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

namespace VietType {

class Compartment : public CComObjectRootEx<CComSingleThreadModel> {
public:
    Compartment() = default;
    Compartment(const Compartment&) = delete;
    Compartment& operator=(const Compartment&) = delete;
    ~Compartment() = default;

    DECLARE_NOT_AGGREGATABLE(Compartment)
    BEGIN_COM_MAP(Compartment)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    _Ret_valid_ ITfCompartment* GetCompartment();
    _Check_return_ HRESULT GetCompartmentSource(_COM_Outptr_ ITfSource** ppSource);

    _Check_return_ _Success_(return == S_OK) HRESULT GetValue(_Out_ long* val);
    HRESULT SetValue(_In_ long val);

    _Check_return_ HRESULT Initialize(_In_ IUnknown* punk, _In_ TfClientId clientid, _In_ const GUID& guidCompartment, _In_ bool global = false);
    HRESULT Uninitialize();

private:
    CComPtr<ITfCompartment> _compartment;
    TfClientId _clientid = TF_CLIENTID_NULL;
};

}
