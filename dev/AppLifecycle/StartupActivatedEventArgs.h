// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.
#pragma once

#include <winrt/Windows.Foundation.h>
#include "ActivatedEventArgsBase.h"
#include "../Common/UriHelpers.h"

namespace winrt::Microsoft::Windows::AppLifecycle::implementation
{
    using namespace winrt::Windows::ApplicationModel::Activation;

    constexpr PCWSTR c_startupTaskContractId = L"Windows.StartupTask";

    class StartupActivatedEventArgs : public winrt::implements<StartupActivatedEventArgs, IStartupTaskActivatedEventArgs,
        ActivatedEventArgsBase, IInternalValueMarshalable>
    {
    public:
        StartupActivatedEventArgs(const winrt::hstring taskId) : m_taskId(taskId)
        {
            m_kind = ActivationKind::StartupTask;
        }

        static winrt::Windows::Foundation::IInspectable Deserialize(winrt::Windows::Foundation::Uri const& uri)
        {
            // Use custom query parameter parser to handle Unicode characters
            auto queryParams = ParseUriQueryParameters(uri);
            auto taskId = GetQueryParamValueByName(queryParams, L"TaskId");
            return make<StartupActivatedEventArgs>(taskId.c_str());
        }

        // IInternalValueMarshalable
        winrt::Windows::Foundation::Uri Serialize()
        {
            auto uri = GenerateEncodedLaunchUri(L"App", c_startupTaskContractId) + L"&TaskId=" + winrt::Windows::Foundation::Uri::EscapeComponent(m_taskId.c_str());
            return winrt::Windows::Foundation::Uri(uri);
        }

        // IStartupTaskActivatedEventArgs
        winrt::hstring TaskId()
        {
            return m_taskId.c_str();
        }

    private:
        winrt::hstring m_taskId;
    };
}

