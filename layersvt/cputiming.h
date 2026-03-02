/* Copyright (c) 2015-2023 The Khronos Group Inc.
 * Copyright (c) 2015-2023 Valve Corporation
 * Copyright (c) 2015-2023 LunarG, Inc.
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

#pragma once

#include <vulkan/vulkan.h>
#include <mutex>
#include <unordered_map>

class CpuTiming {
   public:
    static CpuTiming& Get();

    void SetVkInstance(VkPhysicalDevice phys_dev, VkInstance instance);
    VkInstance GetVkInstance(VkPhysicalDevice phys_dev);

   private:
    std::mutex map_mutex_;
    std::unordered_map<VkPhysicalDevice, VkInstance> vk_instance_map_;
};
