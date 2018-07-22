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
    Compartment() noexcept;
    ~Compartment();

    DECLARE_NOT_AGGREGATABLE(Compartment)
    BEGIN_COM_MAP(Compartment)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

public:
    ITfCompartment* GetCompartment();
    HRESULT GetCompartmentSource(ITfSource** ppSource);

    HRESULT GetValue(long* val);
    HRESULT SetValue(long val);

    HRESULT Initialize(IUnknown* punk, TfClientId clientid, const GUID& guidCompartment, bool global = false);
    HRESULT Uninitialize();

private:
    CComPtr<ITfCompartment> _compartment;
    TfClientId _clientid = TF_CLIENTID_NULL;

private:
    DISALLOW_COPY_AND_ASSIGN(Compartment);
};

}
