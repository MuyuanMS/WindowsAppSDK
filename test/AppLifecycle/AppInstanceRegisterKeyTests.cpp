// Copyright (c) Microsoft Corporation and Contributors.
// Licensed under the MIT License.

#include "pch.h"
#include <testdef.h>
#include "Shared.h"

using namespace WEX::Common;
using namespace WEX::Logging;
using namespace WEX::TestExecution;

using namespace winrt;
using namespace winrt::Microsoft::Windows::AppLifecycle;
using namespace winrt::Windows::ApplicationModel::Activation;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Foundation::Collections;
using namespace winrt::Windows::Management::Deployment;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::System;

namespace Test::AppLifecycle
{
    class AppInstanceRegisterKeyTests
    {
    private:
        const std::wstring c_testKeyName = L"TestKey_ReRegister";

    public:
        BEGIN_TEST_CLASS(AppInstanceRegisterKeyTests)
            TEST_CLASS_PROPERTY(L"ThreadingModel", L"MTA")
            TEST_CLASS_PROPERTY(L"RunAs:Class", L"RestrictedUser")
        END_TEST_CLASS()

        TEST_CLASS_SETUP(ClassInit)
        {
            ::Test::Bootstrap::Setup();
            return true;
        }

        TEST_CLASS_CLEANUP(ClassUninit)
        {
            ::Test::Bootstrap::Cleanup();
            return true;
        }

        TEST_METHOD(UnregisterAndReRegisterKey)
        {
            // First register a key
            auto instance = AppInstance::FindOrRegisterForKey(c_testKeyName);
            VERIFY_IS_NOT_NULL(instance);
            VERIFY_IS_TRUE(instance.IsCurrent());
            VERIFY_ARE_EQUAL(instance.Key(), c_testKeyName);

            // Unregister the key
            instance.UnregisterKey();
            VERIFY_ARE_EQUAL(instance.Key(), L"");

            // Re-register the same key
            auto reRegisteredInstance = AppInstance::FindOrRegisterForKey(c_testKeyName);
            VERIFY_IS_NOT_NULL(reRegisteredInstance);
            VERIFY_IS_TRUE(reRegisteredInstance.IsCurrent());
            VERIFY_ARE_EQUAL(reRegisteredInstance.Key(), c_testKeyName);
        }
    };
}