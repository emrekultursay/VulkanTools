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

#ifndef __ANDROID__
#include <iostream>
#endif
#include "debug_marker.h"
#include "debug_marker_perfetto.h"
#include "perfetto/perfetto.h"

DebugMarker& DebugMarker::Get() {
    static DebugMarker instance;
    return instance;
}

void DebugMarker::SetVkInstance(VkPhysicalDevice phys_dev, VkInstance instance) {
    std::lock_guard<std::mutex> lock(mutex_);
    vk_instance_map_[phys_dev] = instance;
}

VkInstance DebugMarker::GetVkInstance(VkPhysicalDevice phys_dev) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = vk_instance_map_.find(phys_dev);
    if (it != vk_instance_map_.end()) return it->second;
    return VK_NULL_HANDLE;
}

void DebugMarker::SetDebugObjectName(uint64_t device, int32_t type, uint64_t handle, const char* name) {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string name_str = name ? name : "NULL";
    debug_object_names_[std::make_pair(type, handle)] = DebugObjectName(device, type, handle, name_str);
#ifdef __ANDROID__
    __android_log_print(ANDROID_LOG_WARN, "DebugMarker", "DebugMarker::SetDebugObjectName");
    perfetto::TrackEvent::Trace([device, type, handle, name_str](perfetto::TrackEvent::TraceContext ctx) {
        auto packet = ctx.NewTracePacket();
        packet->set_timestamp(perfetto::base::GetBootTimeNs().count());
        auto event = packet->set_vulkan_api_event()->set_vk_debug_utils_object_name();
        event->set_vk_device(device);
        event->set_object_type(type);
        event->set_object(handle);
        event->set_object_name(name_str.c_str());
    });
#else
    std::cout << "SetDebugObjectName: [device: " << device << ", type: " << type << ", handle: " << handle << "] " << name_str << std::endl;
#endif
}

void DebugMarker::EmitAllDebugMarkers() {
    std::lock_guard<std::mutex> lock(mutex_);
#ifdef __ANDROID__
    for (const auto& entry : debug_object_names_) {
        __android_log_print(ANDROID_LOG_WARN, "DebugMarker", "DebugMarker::EmitAllDebugMarkers - Loop");
        const auto& marker = entry.second;
        uint64_t device = marker.vk_device;
        int32_t type = marker.object_type;
        uint64_t handle = marker.handle;
        std::string name_str = marker.name;
        
        perfetto::TrackEvent::Trace([device, type, handle, name_str](perfetto::TrackEvent::TraceContext ctx) {
            __android_log_print(ANDROID_LOG_WARN, "DebugMarker", "DebugMarker::EmitAllDebugMarkers - Loop - TrackEvent::Trace");
            auto packet = ctx.NewTracePacket();
            packet->set_timestamp(perfetto::base::GetBootTimeNs().count());
            auto event = packet->set_vulkan_api_event()->set_vk_debug_utils_object_name();
            event->set_vk_device(device);
            event->set_object_type(type);
            event->set_object(handle);
            event->set_object_name(name_str.c_str());
        });
    }
#else
    std::cout << "EmitAllDebugMarkers: " << debug_object_names_.size() << std::endl;
#endif
}

void DebugMarker::Clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    vk_instance_map_.clear();
    debug_object_names_.clear();
}

bool DebugMarker::HasDebugObjectName(int32_t type, uint64_t handle, const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = debug_object_names_.find(std::make_pair(type, handle));
    if (it == debug_object_names_.end()) return false;
    return it->second.name == name;
}
