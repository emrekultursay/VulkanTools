/* Copyright (C) 2026 Google Inc.
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
#include "../debug_marker.h"
#include <vulkan/vulkan_core.h>
#include <gtest/gtest.h>
#include <stdlib.h>

static const char* kLayerName = "VK_LAYER_GOOGLE_DebugMarker";

class DebugMarkerTests : public VkTestFramework {
   public:
    ~DebugMarkerTests(){};

    static void SetUpTestSuite() {}
    static void TearDownTestSuite(){};
};

TEST_F(DebugMarkerTests, CombinedTest) {
    TEST_DESCRIPTION("Combined test for DebugMarker layer");

    // Capture stdout to verify layer output
    testing::internal::CaptureStdout();

    layer_test::VulkanInstanceBuilder inst_builder;
    // Enable DebugUtils Vulkan extension.
    inst_builder.AddExtension("VK_EXT_debug_utils");
    VkResult err = inst_builder.Init(kLayerName);
    ASSERT_EQ(err, VK_SUCCESS);

    // Get physical device
    VkPhysicalDevice phys_dev = VK_NULL_HANDLE;
    err = inst_builder.GetPhysicalDevice(&phys_dev);
    EXPECT_EQ(err, VK_SUCCESS);
    EXPECT_NE(phys_dev, VK_NULL_HANDLE);



    // Get instance.
    VkInstance instance = inst_builder.GetInstance();
    EXPECT_NE(instance, VK_NULL_HANDLE);

    // Create device
    VkDevice device = VK_NULL_HANDLE;
    VkDeviceCreateInfo device_create_info = {VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};
    VkDeviceQueueCreateInfo queue_create_info = {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO};
    queue_create_info.queueFamilyIndex = 0;
    queue_create_info.queueCount = 1;
    float queue_priority = 1.0f;
    queue_create_info.pQueuePriorities = &queue_priority;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.pQueueCreateInfos = &queue_create_info;

    // Enable DebugMarker Vulkan extension.
    const char* device_extensions[] = {"VK_EXT_debug_marker"};
    device_create_info.enabledExtensionCount = 1;
    device_create_info.ppEnabledExtensionNames = device_extensions;

    err = vkCreateDevice(phys_dev, &device_create_info, nullptr, &device);
    EXPECT_EQ(err, VK_SUCCESS);
    EXPECT_NE(device, VK_NULL_HANDLE);

    // Verify that GetInstanceProcAddr returns layer functions
    PFN_vkVoidFunction pfnSetDebugUtilsObjectNameEXT = vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    EXPECT_NE(pfnSetDebugUtilsObjectNameEXT, nullptr);

    // Verify Vulkan layer intercepts vkSetDebugUtilsObjectNameEXT correctly.

    // Set instance name.
    PFN_vkSetDebugUtilsObjectNameEXT real_vkSetDebugUtilsObjectNameEXT = (PFN_vkSetDebugUtilsObjectNameEXT)pfnSetDebugUtilsObjectNameEXT;
    VkDebugUtilsObjectNameInfoEXT name_info = {VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT};
    name_info.objectType = VK_OBJECT_TYPE_INSTANCE;
    name_info.objectHandle = (uint64_t)instance;
    name_info.pObjectName = "MyInstance";
    err = real_vkSetDebugUtilsObjectNameEXT(device, &name_info);
    ASSERT_EQ(err, VK_SUCCESS);

    // Verify the method was intercepted by the Vulkan layer.
    std::string output = testing::internal::GetCapturedStdout();
    std::cerr << "===stdout: " << std::endl << output << std::endl << "===" << std::endl;
    EXPECT_NE(output.find("SetDebugObjectName:"), std::string::npos);

    // Override new name.
    testing::internal::CaptureStdout();
    name_info.pObjectName = "MyInstanceRenamed";
    err = real_vkSetDebugUtilsObjectNameEXT(device, &name_info);
    EXPECT_EQ(err, VK_SUCCESS);

    // Verify the method was intercepted by the Vulkan layer.
    output = testing::internal::GetCapturedStdout();
    std::cerr << "===stdout: " << std::endl << output << std::endl << "===" << std::endl;
    EXPECT_NE(output.find("SetDebugObjectName:"), std::string::npos);


    // Verify Vulkan layer intercepts vkDebugMarkerSetObjectNameEXT correctly.
    PFN_vkDebugMarkerSetObjectNameEXT real_vkDebugMarkerSetObjectNameEXT =
        (PFN_vkDebugMarkerSetObjectNameEXT)vkGetInstanceProcAddr(instance, "vkDebugMarkerSetObjectNameEXT");
    ASSERT_NE(real_vkDebugMarkerSetObjectNameEXT, nullptr);

    VkDebugMarkerObjectNameInfoEXT marker_name_info = {VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT};
    marker_name_info.objectType = VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT;
    marker_name_info.object = (uint64_t)instance;
    marker_name_info.pObjectName = "MyInstanceThirdName";

    testing::internal::CaptureStdout();
    err = real_vkDebugMarkerSetObjectNameEXT(device, &marker_name_info);
    EXPECT_EQ(err, VK_SUCCESS);

    // Verify the method was intercepted by the Vulkan layer.
    output = testing::internal::GetCapturedStdout();
    std::cerr << "===stdout: " << std::endl << output << std::endl << "===" << std::endl;
    EXPECT_NE(output.find("SetDebugObjectName:"), std::string::npos);


    // TODO: Is it possible to trigger the EmitAllDebugMarkers somehow? That
    // would add to the test coverage.

    // Cleanup: Required for proper exit.
    vkDestroyDevice(device, nullptr);
}


