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

template <typename... Args>
class EditSession :
    public CComObjectRootEx<CComSingleThreadModel>,
    public ITfEditSession {
public:
    using funtype = HRESULT(*)(TfEditCookie ec, Args... args);

    EditSession() noexcept {
    }
    ~EditSession() {
    }

    DECLARE_NOT_AGGREGATABLE(EditSession)
    BEGIN_COM_MAP(EditSession)
        COM_INTERFACE_ENTRY(ITfEditSession)
    END_COM_MAP()
    DECLARE_PROTECT_FINAL_CONSTRUCT()

    // Inherited via ITfEditSession
    virtual STDMETHODIMP DoEditSession(_In_ TfEditCookie ec) {
        auto args_ec = std::tuple_cat(std::make_tuple(ec), _args);
        return std::apply(_callback, args_ec);
    }

    void Initialize(_In_ funtype callback, Args... args) {
        _callback = callback;
        _args = std::make_tuple(args...);
    }

private:
    funtype _callback = nullptr;
    std::tuple<Args...> _args;

private:
    DISALLOW_COPY_AND_ASSIGN(EditSession);
};

}
