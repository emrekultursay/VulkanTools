/*
 * Copyright (c) 2024 Your Company
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "layer_test_helper.h"

#include <vulkan/vulkan_core.h>

#include <gtest/gtest.h>

static const char* kLayerName = "CPUTiming";

class CpuTimingTests : public VkTestFramework {
   public:
    ~CpuTimingTests(){};

    static void SetUpTestSuite() {}
    static void TearDownTestSuite(){};
};

TEST_F(CpuTimingTests, InitLayer) {
    TEST_DESCRIPTION("Test Creating a Vulkan Instance with CPUTiming layer");

    // Capture stdout to verify layer output
    testing::internal::CaptureStdout();

    layer_test::VulkanInstanceBuilder inst_builder;
    VkResult err = inst_builder.Init(kLayerName);
    EXPECT_EQ(err, VK_SUCCESS);

    VkPhysicalDevice phys_dev;
    err = inst_builder.GetPhysicalDevice(&phys_dev);
    // It's possible there are no physical devices, so we just check it didn't crash.
    // If err is VK_SUCCESS, we found one.
    if (err == VK_SUCCESS) {
        EXPECT_NE(phys_dev, nullptr);
    }
    
    // Explicitly destroy instance to ensure all timers are flushed/destructed
    inst_builder.Reset();

    std::string output = testing::internal::GetCapturedStdout();
    
    // Print captured output to stderr so it's visible in test logs
    std::cerr << "Captured Output:\n" << output << "\n";

    // Verify that we saw creating instance timing
    // Expected output format: "vkCreateInstance: <duration> ns"
    EXPECT_NE(output.find("CPUTiming/Other vkCreateInstance:"), std::string::npos);
    EXPECT_NE(output.find("ns"), std::string::npos);
    
    // If we found a physical device, we should see enumeration timing too
    if (err == VK_SUCCESS) {
        EXPECT_NE(output.find("vkEnumeratePhysicalDevices:"), std::string::npos);
    }
}
