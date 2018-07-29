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

#include <atlbase.h>

template <typename T>
_Check_return_ HRESULT CreateInstance2(_Outptr_ T** ppout) {
    ATL::CComObject<T>* p;
    HRESULT hr = ATL::CComObject<T>::CreateInstance(&p);
    if (SUCCEEDED(hr)) {
        p->AddRef();
        *ppout = p;
    }
    return hr;
}

template <typename TFrom, typename TTo>
_Check_return_ HRESULT QueryInterface2(_In_ TFrom* from, _Outptr_ TTo** to) {
    return from->QueryInterface(__uuidof(TTo), reinterpret_cast<void**>(to));
}
