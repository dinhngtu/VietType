// SPDX-FileCopyrightText: Copyright (c) 2018 Dinh Ngoc Tu
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include "Common.h"

namespace VietType {

template <typename... Args>
class EditSession : public CComObjectRootEx<CComSingleThreadModel>, public ITfEditSession {
public:
    using callback_type = HRESULT (*)(TfEditCookie ec, Args... args);

    EditSession() = default;
    EditSession(const EditSession&) = delete;
    EditSession& operator=(const EditSession&) = delete;
    ~EditSession() = default;

    DECLARE_NO_REGISTRY()
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

    HRESULT Initialize(_In_ callback_type callback, Args... args) {
        _callback = callback;
        _args = std::make_tuple(args...);
        return S_OK;
    }

    // dummy method
    HRESULT Uninitialize() {
        return S_OK;
    }

private:
    callback_type _callback = nullptr;
    std::tuple<Args...> _args;
};

} // namespace VietType
