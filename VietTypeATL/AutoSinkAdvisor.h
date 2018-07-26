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

#include "SinkAdvisor.h"

template <typename TSink>
class AutoSinkAdvisor {
public:
    ~AutoSinkAdvisor() {
        HRESULT hr = sink.Unadvise();
        hr;
        DBG_HRESULT_CHECK(hr, L"SinkAdvisor<%s>::Unadvise failed", typeid(TSink).name());
    }

    static std::tuple<std::unique_ptr<AutoSinkAdvisor<TSink>>, HRESULT> AdviseSink(_In_ IUnknown* source, _In_ TSink* sink) {
        auto autosink = std::make_unique<AutoSinkAdvisor<TSink>>();
        HRESULT hr = autosink->sink.Advise(source, sink);
        return std::make_tuple(std::move(autosink), hr);
    }

private:
    SinkAdvisor<TSink> sink;
};

template <typename TSink>
class AutoSingleSinkAdvisor {
public:
    ~AutoSingleSinkAdvisor() {
        HRESULT hr = sink.Unadvise();
        hr;
        DBG_HRESULT_CHECK(hr, L"SingleSinkAdvisor<%s>::Unadvise failed", typeid(TSink).name());
    }

    static std::tuple<std::unique_ptr<AutoSingleSinkAdvisor<TSink>>, HRESULT> AdviseSink(_In_ IUnknown* source, _In_ TfClientId client_id, _In_ TSink* sink) {
        auto autosink = std::make_unique<AutoSingleSinkAdvisor<TSink>>();
        HRESULT hr = autosink->sink.Advise(source, client_id, sink);
        return std::make_tuple(std::move(autosink), hr);
    }

private:
    SingleSinkAdvisor<TSink> sink;
};
