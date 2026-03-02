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

#ifndef LAYERSVT_TIMER_H
#define LAYERSVT_TIMER_H

#include "perfetto/perfetto_helpers.h"

#ifndef __ANDROID__
#include <chrono>
#include <iostream>
#endif



class Timer {
  public:
    explicit Timer(CPUTimingCategory category, const char* name) : cat_(category) {
#ifndef __ANDROID__
        name_ = name;
        start_ = std::chrono::high_resolution_clock::now();
#endif
        // Perfetto categories must be static strings known at compile time or registered
        // via DynamicCategory. Using DynamicCategory is significantly slower, so we use
        // static strings instead. Note that we cannot even use a `get_category_name(cat_)`
        // helper function here since it would not be a compile-time constexpr.
        switch (cat_) {
            case CPUTimingCategory::VkInstance: {
                TRACE_EVENT_BEGIN("CPUTiming/VkInstance", perfetto::StaticString(name));
                break;
            }
            case CPUTimingCategory::VkPhysicalDevice: {
                TRACE_EVENT_BEGIN("CPUTiming/VkPhysicalDevice", perfetto::StaticString(name));
                break;
            }
            case CPUTimingCategory::VkDevice: {
                TRACE_EVENT_BEGIN("CPUTiming/VkDevice", perfetto::StaticString(name));
                break;
            }
            case CPUTimingCategory::VkQueue: {
                TRACE_EVENT_BEGIN("CPUTiming/VkQueue", perfetto::StaticString(name));
                break;
            }
            case CPUTimingCategory::VkCommandBuffer: {
                TRACE_EVENT_BEGIN("CPUTiming/VkCommandBuffer", perfetto::StaticString(name));
                break;
            }
            default: {
                TRACE_EVENT_BEGIN("CPUTiming/Other", perfetto::StaticString(name));
                break;
            }
        }
    }

    ~Timer() {
        switch (cat_) {
            case CPUTimingCategory::VkInstance: {
                TRACE_EVENT_END("CPUTiming/VkInstance");
                break;
            }
            case CPUTimingCategory::VkPhysicalDevice: {
                TRACE_EVENT_END("CPUTiming/VkPhysicalDevice");
                break;
            }
            case CPUTimingCategory::VkDevice: {
                TRACE_EVENT_END("CPUTiming/VkDevice");
                break;
            }
            case CPUTimingCategory::VkQueue: {
                TRACE_EVENT_END("CPUTiming/VkQueue");
                break;
            }
            case CPUTimingCategory::VkCommandBuffer: {
                TRACE_EVENT_END("CPUTiming/VkCommandBuffer");
                break;
            }
            default: {
                TRACE_EVENT_END("CPUTiming/Other");
                break;
            }
        }
#ifndef __ANDROID__
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
        const char* cat_str = "CPUTiming/Other";
        switch (cat_) {
            case CPUTimingCategory::VkInstance: cat_str = "CPUTiming/VkInstance"; break;
            case CPUTimingCategory::VkPhysicalDevice: cat_str = "CPUTiming/VkPhysicalDevice"; break;
            case CPUTimingCategory::VkDevice: cat_str = "CPUTiming/VkDevice"; break;
            case CPUTimingCategory::VkQueue: cat_str = "CPUTiming/VkQueue"; break;
            case CPUTimingCategory::VkCommandBuffer: cat_str = "CPUTiming/VkCommandBuffer"; break;
            default: break;
        }
        std::cout << cat_str << " " << name_ << ": " << duration << " ns" << std::endl;
#endif
    }

  private:
    CPUTimingCategory cat_;
#ifndef __ANDROID__
    const char* name_;
#endif
#ifndef __ANDROID__
    std::chrono::time_point<std::chrono::high_resolution_clock> start_;
#endif
};

#endif // LAYERSVT_TIMER_H
