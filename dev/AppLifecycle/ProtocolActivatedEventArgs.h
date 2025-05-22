// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.
#pragma once

#include <winrt/Windows.Foundation.h>
#include "ActivatedEventArgsBase.h"
#include "../Common/UriHelpers.h"

namespace winrt::Microsoft::Windows::AppLifecycle::implementation
{
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::ApplicationModel::Activation;

    constexpr PCWSTR c_protocolContractId = L"Windows.Protocol";

    class ProtocolActivatedEventArgs : public winrt::implements<ProtocolActivatedEventArgs, IProtocolActivatedEventArgs, ActivatedEventArgsBase,
        IInternalValueMarshalable>
    {
    public:
        ProtocolActivatedEventArgs(const winrt::hstring uri) : m_uri(winrt::Windows::Foundation::Uri(uri))
        {
            m_kind = ActivationKind::Protocol;
        }

        static winrt::Windows::Foundation::IInspectable Deserialize(winrt::Windows::Foundation::Uri const& uri)
        {
            // Use custom query parameter parser to handle Unicode characters
            auto queryParams = ParseUriQueryParameters(uri);
            auto args = GetQueryParamValueByName(queryParams, L"Uri");
            return make<ProtocolActivatedEventArgs>(args.c_str());
        }

        // IInternalValueMarshalable
        winrt::Windows::Foundation::Uri Serialize()
        {
            auto uri = GenerateEncodedLaunchUri(L"App", c_protocolContractId) + L"&Uri=" + winrt::Windows::Foundation::Uri::EscapeComponent(m_uri.AbsoluteUri());
            return winrt::Windows::Foundation::Uri(uri);
        }

        // IProtocolActivatedEventArgs
        winrt::Windows::Foundation::Uri Uri()
        {
            return m_uri;
        }

    private:
        winrt::Windows::Foundation::Uri m_uri{nullptr};
    };
}
