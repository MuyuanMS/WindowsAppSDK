// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.
#pragma once

#include <string>
#include <map>
#include <vector>
#include <winrt/Windows.Foundation.h>

namespace winrt::Microsoft::Windows::AppLifecycle::implementation
{
    // Custom query parameter parser that can handle Unicode characters
    inline std::map<std::wstring, std::wstring> ParseUriQueryParameters(const winrt::Windows::Foundation::Uri& uri)
    {
        std::map<std::wstring, std::wstring> queryParams;
        
        // Get the raw query string
        std::wstring queryString = uri.Query().c_str();
        
        // Remove the leading '?' if present
        if (!queryString.empty() && queryString[0] == L'?')
        {
            queryString = queryString.substr(1);
        }
        
        // Split the query string by '&'
        size_t startPos = 0;
        size_t endPos = queryString.find(L'&');
        
        while (startPos < queryString.length())
        {
            std::wstring keyValuePair;
            
            if (endPos == std::wstring::npos)
            {
                keyValuePair = queryString.substr(startPos);
                startPos = queryString.length();
            }
            else
            {
                keyValuePair = queryString.substr(startPos, endPos - startPos);
                startPos = endPos + 1;
                endPos = queryString.find(L'&', startPos);
            }
            
            // Split the key-value pair by '='
            size_t separatorPos = keyValuePair.find(L'=');
            if (separatorPos != std::wstring::npos)
            {
                std::wstring key = keyValuePair.substr(0, separatorPos);
                std::wstring value = keyValuePair.substr(separatorPos + 1);
                
                // Unescape the value to handle Unicode characters
                if (!value.empty())
                {
                    value = winrt::Windows::Foundation::Uri::UnescapeComponent(value).c_str();
                }
                
                queryParams[key] = value;
            }
        }
        
        return queryParams;
    }
    
    // Helper function to get a value by name from the query parameters
    inline std::wstring GetQueryParamValueByName(const std::map<std::wstring, std::wstring>& queryParams, const std::wstring& name)
    {
        auto it = queryParams.find(name);
        if (it != queryParams.end())
        {
            return it->second;
        }
        return L"";
    }
}