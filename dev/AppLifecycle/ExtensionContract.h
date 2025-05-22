// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.
#pragma once

#include "winrt/Microsoft.Windows.AppLifecycle.h"
#include "LaunchActivatedEventArgs.h"
#include "ProtocolActivatedEventArgs.h"
#include "FileActivatedEventArgs.h"
#include "StartupActivatedEventArgs.h"
#include "PushNotificationManager.h"
#include "AppNotificationManager.h"
#include "../Common/UriHelpers.h"

namespace winrt::Microsoft::Windows::AppLifecycle::implementation
{
    // This array holds the mapping between a class factory and it's extension contract Id.
    struct ExtensionMap
    {
        ExtendedActivationKind kind;
        PCWSTR contractId;
        winrt::Windows::Foundation::IInspectable(*factory)(winrt::Windows::Foundation::Uri const& uri);
    };

    constexpr ExtensionMap c_extensionMap[] =
    {
        { ExtendedActivationKind::Launch, c_launchContractId, &LaunchActivatedEventArgs::Deserialize },
        { ExtendedActivationKind::File, c_fileContractId, &FileActivatedEventArgs::Deserialize },
        { ExtendedActivationKind::Protocol, c_protocolContractId, &ProtocolActivatedEventArgs::Deserialize },
        { ExtendedActivationKind::StartupTask, c_startupTaskContractId, &StartupActivatedEventArgs::Deserialize },
        { ExtendedActivationKind::Push, c_pushContractId, &winrt::Microsoft::Windows::PushNotifications::implementation::PushNotificationManager::PushDeserialize },
        { ExtendedActivationKind::AppNotification, c_appNotificationContractId, &winrt::Microsoft::Windows::AppNotifications::implementation::AppNotificationManager::AppNotificationDeserialize },
    };

    inline bool IsEncodedLaunch(winrt::Windows::Foundation::Uri const& uri)
    {
        return CompareStringOrdinal(uri.SchemeName().c_str(), -1, c_encodedLaunchSchemeName, -1, TRUE) == CSTR_EQUAL;
    }

    inline std::tuple<ExtendedActivationKind, winrt::Windows::Foundation::IInspectable> DecodeActivatedEventArgs(winrt::Windows::Foundation::Uri const& uri)
    {
        // Use custom query parameter parser to handle Unicode characters
        auto queryParams = ParseUriQueryParameters(uri);
        
        // Check for ContractId in the query parameters
        auto contractIdIt = queryParams.find(c_contractIdKeyName);
        if (contractIdIt != queryParams.end())
        {
            auto contractId = contractIdIt->second;
            
            for (const auto& extension : c_extensionMap)
            {
                if (CompareStringOrdinal(contractId.c_str(), -1, extension.contractId, -1, TRUE) == CSTR_EQUAL)
                {
                    return { extension.kind, extension.factory(uri) };
                }
            }
        }
        
        // If we have a File parameter in the query, this is likely a file activation
        // This is a fallback for when the ContractId parsing might fail
        auto fileValue = GetQueryParamValueByName(queryParams, L"File");
        auto verbValue = GetQueryParamValueByName(queryParams, L"Verb");
        
        if (!fileValue.empty() && !verbValue.empty())
        {
            for (const auto& extension : c_extensionMap)
            {
                if (CompareStringOrdinal(extension.contractId, -1, c_fileContractId, -1, TRUE) == CSTR_EQUAL)
                {
                    return { extension.kind, extension.factory(uri) };
                }
            }
        }
        
        return { ExtendedActivationKind::Protocol, nullptr };
    }
}
