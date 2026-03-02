/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
 * Copyright (C) 2015-2016 Google Inc.
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
 *
 */

// Implementation file for specifically implemented functions

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#include "generated/cputiming_implementation.h"
#include "timer.h"
#include "cputiming.h"
#include "perfetto/perfetto_helpers.h"
#include "vk_layer_table.h"

#if defined(__GNUC__) && __GNUC__ >= 4
#define EXPORT_FUNCTION __attribute__((visibility("default")))
#elif defined(__SUNPRO_C) && (__SUNPRO_C >= 0x590)
#define EXPORT_FUNCTION __attribute__((visibility("default")))
#else
#define EXPORT_FUNCTION
#endif

#include <mutex>

static std::once_flag g_perfetto_init_flag;

extern "C" {

VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(const VkInstanceCreateInfo* pCreateInfo, const VkAllocationCallbacks* pAllocator,
                                                VkInstance* pInstance) {
    std::call_once(g_perfetto_init_flag, []() { InitializePerfetto(); });
    Timer timer(CPUTimingCategory::Other, "vkCreateInstance");



    // Get the function pointer
    VkLayerInstanceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    assert(chain_info->u.pLayerInfo != 0);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    assert(fpGetInstanceProcAddr != 0);
    PFN_vkCreateInstance fpCreateInstance = (PFN_vkCreateInstance)fpGetInstanceProcAddr(NULL, "vkCreateInstance");
    if (fpCreateInstance == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Call the function and create the dispatch table
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    VkResult result = fpCreateInstance(pCreateInfo, pAllocator, pInstance);
    if (result == VK_SUCCESS) {
        initInstanceTable(*pInstance, fpGetInstanceProcAddr);
    }

    return result;
}
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo* pCreateInfo,
                                              const VkAllocationCallbacks* pAllocator, VkDevice* pDevice) {
    Timer timer(CPUTimingCategory::VkPhysicalDevice, "vkCreateDevice");

    // Get the function pointer
    VkLayerDeviceCreateInfo* chain_info = get_chain_info(pCreateInfo, VK_LAYER_LINK_INFO);
    assert(chain_info->u.pLayerInfo != 0);
    PFN_vkGetInstanceProcAddr fpGetInstanceProcAddr = chain_info->u.pLayerInfo->pfnNextGetInstanceProcAddr;
    PFN_vkGetDeviceProcAddr fpGetDeviceProcAddr = chain_info->u.pLayerInfo->pfnNextGetDeviceProcAddr;
    VkInstance vk_instance = CpuTiming::Get().GetVkInstance(physicalDevice);
    PFN_vkCreateDevice fpCreateDevice = (PFN_vkCreateDevice)fpGetInstanceProcAddr(vk_instance, "vkCreateDevice");
    if (fpCreateDevice == NULL) {
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Call the function and create the dispatch table
    chain_info->u.pLayerInfo = chain_info->u.pLayerInfo->pNext;
    VkResult result = fpCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
    if (result == VK_SUCCESS) {
        initDeviceTable(*pDevice, fpGetDeviceProcAddr);
    }

    return result;
}

extern "C" {

EXPORT_FUNCTION VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(const char* pLayerName,
                                                                                      uint32_t* pPropertyCount,
                                                                                      VkExtensionProperties* pProperties) {
    Timer timer(CPUTimingCategory::Other, "vkEnumerateInstanceExtensionProperties");
    return util_GetExtensionProperties(0, NULL, pPropertyCount, pProperties);
}

EXPORT_FUNCTION VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(uint32_t* pPropertyCount,
                                                                                  VkLayerProperties* pProperties) {
    Timer timer(CPUTimingCategory::Other, "vkEnumerateInstanceLayerProperties");
    static const VkLayerProperties layerProperties[] = {{
        "CPUTiming",
        VK_MAKE_VERSION(1, 4, VK_HEADER_VERSION),  // specVersion
        VK_MAKE_VERSION(0, 2, 0),                  // implementationVersion
        "layer: CPUTiming",
    }};

    return util_GetLayerProperties(ARRAY_SIZE(layerProperties), layerProperties, pPropertyCount, pProperties);
}

EXPORT_FUNCTION VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice,
                                                                                uint32_t* pPropertyCount,
                                                                                VkLayerProperties* pProperties) {
    Timer timer(CPUTimingCategory::VkPhysicalDevice, "vkEnumerateDeviceLayerProperties");
    static const VkLayerProperties layerProperties[] = {{
        "CPUTiming",
        VK_MAKE_VERSION(1, 4, VK_HEADER_VERSION),
        VK_MAKE_VERSION(0, 2, 0),
        "layer: CPUTiming",
    }};

    return util_GetLayerProperties(ARRAY_SIZE(layerProperties), layerProperties, pPropertyCount, pProperties);
}
}
