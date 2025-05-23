// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.
#pragma once

#include "ActivatedEventArgsBase.h"

namespace winrt::Microsoft::Windows::AppLifecycle::implementation
{
    using namespace winrt::Windows::Foundation::Collections;
    using namespace winrt::Windows::ApplicationModel::Activation;
    using namespace winrt::Windows::Storage;

    constexpr inline PCWSTR c_fileContractId = L"Windows.File";

    class FileActivatedEventArgs : public winrt::implements<FileActivatedEventArgs, IFileActivatedEventArgs, ActivatedEventArgsBase,
        IInternalValueMarshalable>
    {
    public:
        FileActivatedEventArgs(const winrt::hstring verb, const winrt::hstring file, const bool supportCommandTemplates = false)
        {
            if (verb.empty())
            {
                throw std::invalid_argument("verb");
            }

            if (file.empty())
            {
                throw std::invalid_argument("file");
            }

            m_supportCommandTemplates = supportCommandTemplates;
            m_kind = ActivationKind::File;
            m_verb = std::move(verb);
            m_path = std::move(file);
            m_files = winrt::single_threaded_vector<IStorageItem>();

            // There is a scenario where we just want to create an object to serialize it.  In that situation
            // skipping verification allows for template variables to be used.  Example: %1 may be used when
            // using serialization to generate the encoded arguments for a file type association.
            if (!m_supportCommandTemplates)
            {
                // Currently we only support one file in the array, because the
                // activation method forces a new process for each item in the array.
                m_files.Append(StorageFile::GetFileFromPathAsync(m_path.c_str()).get());
            }
        }

        static winrt::Windows::Foundation::IInspectable Deserialize(winrt::Windows::Foundation::Uri const& uri)
        {
            winrt::hstring verb;
            winrt::hstring file;
            
            // Try to get parameters using standard QueryParsed
            try
            {
                auto query = uri.QueryParsed();
                verb = query.GetFirstValueByName(L"Verb");
            }
            catch (...)
            {
                // Fall back to manual extraction if QueryParsed fails
                verb = ExtractQueryParameterValue(uri.Query(), L"Verb");
            }
            
            // For File parameter, always use direct extraction since:
            // 1. It can contain Unicode characters which QueryParsed() might not handle correctly
            // 2. It can contain & characters in the filename
            // 3. It's always the last parameter in the URI (by convention in the Serialize method)
            file = ExtractFileParameterValue(uri.Query());
            
            if (!verb.empty() && !file.empty())
            {
                return make<FileActivatedEventArgs>(verb, file);
            }
            
            throw winrt::hresult_invalid_argument(L"Missing required parameters");
        }
        
        // Helper method to extract the File parameter value, assuming File is the last parameter
        static winrt::hstring ExtractFileParameterValue(winrt::hstring const& query)
        {
            std::wstring queryStr = query.c_str();
            const std::wstring fileParamPrefix = L"File=";
            
            // Find File parameter
            auto filePos = queryStr.find(fileParamPrefix);
            if (filePos != std::wstring::npos)
            {
                // Extract everything after "File=" to the end
                auto fileValue = queryStr.substr(filePos + fileParamPrefix.length());
                
                // Unescape to handle Unicode characters
                if (!fileValue.empty())
                {
                    return winrt::Windows::Foundation::Uri::UnescapeComponent(fileValue);
                }
            }
            
            return L"";
        }
        
        // Helper method to extract a generic query parameter value 
        static winrt::hstring ExtractQueryParameterValue(winrt::hstring const& query, std::wstring const& paramName)
        {
            std::wstring queryStr = query.c_str();
            std::wstring paramPrefix = paramName + L"=";
            
            // Find parameter
            auto paramPos = queryStr.find(paramPrefix);
            if (paramPos != std::wstring::npos)
            {
                // Extract from start position to next & or end
                auto startPos = paramPos + paramPrefix.length();
                auto endPos = queryStr.find(L'&', startPos);
                
                std::wstring paramValue;
                if (endPos != std::wstring::npos)
                {
                    paramValue = queryStr.substr(startPos, endPos - startPos);
                }
                else
                {
                    paramValue = queryStr.substr(startPos);
                }
                
                // Unescape the value
                if (!paramValue.empty())
                {
                    return winrt::Windows::Foundation::Uri::UnescapeComponent(paramValue);
                }
            }
            
            return L"";
        }

        // IInternalValueMarshalable
        winrt::Windows::Foundation::Uri Serialize()
        {
            auto uri = GenerateEncodedLaunchUri(L"App", c_fileContractId);
            uri += L"&Verb=" + winrt::Windows::Foundation::Uri::EscapeComponent(m_verb.c_str());

            std::wstring path;

            if (m_supportCommandTemplates)
            {
                // Don't escape command template files as they could be %1.
                path = m_path.c_str();
            }
            else
            {
                path = winrt::Windows::Foundation::Uri::EscapeComponent(m_path.c_str());
            }

            uri += std::wstring(L"&File=") + path;

            return winrt::Windows::Foundation::Uri(uri);
        }

        // IFileActivatedEventArgs
        IVectorView<IStorageItem> Files()
        {
            return m_files.GetView();
        }

        winrt::hstring Verb()
        {
            return m_verb.c_str();
        }

    private:
        bool m_supportCommandTemplates;
        winrt::hstring m_verb;
        winrt::hstring m_path;
        IVector<IStorageItem> m_files;
    };
}
