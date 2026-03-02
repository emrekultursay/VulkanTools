#!/usr/bin/python3 -i
#
# Copyright (c) 2015-2026 Valve Corporation
# Copyright (c) 2015-2026 LunarG, Inc.
# Copyright (c) 2015-2016, 2019, 2021 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from base_generator import BaseGenerator
from vulkan_object import Command

# These commands are manually implemented in cputiming_handwritten_functions.h
# or cputiming_handwritten_dispatch.cpp
MANUALLY_IMPLEMENTED_COMMANDS = [
    'vkCreateInstance',
    'vkCreateDevice',
    'vkEnumerateInstanceExtensionProperties',
    'vkEnumerateInstanceLayerProperties',
    'vkEnumerateInstanceVersion',
    'vkEnumerateDeviceLayerProperties',
    'vkEnumerateDeviceExtensionProperties',
    'vkGetInstanceProcAddr',
    'vkGetDeviceProcAddr',
]

class PlatformGuardHelper():
    """Used to elide platform guards together, so redundant #endif then #ifdefs are removed
    Note - be sure to call add_guard(None) when done to add a trailing #endif if needed
    """
    def __init__(self):
        self.current_guard = None

    def add_guard(self, generator, guard):
        if self.current_guard != guard and self.current_guard is not None:
            generator.write(f'#endif  // {self.current_guard}')
        if self.current_guard != guard and guard is not None:
            generator.write(f'#if defined({guard})')
        self.current_guard = guard

# Returns the command's parameters declaration. ex) uint32_t foo, float bar, char* baz
def command_param_declaration_text(command : Command):
    return ', '.join(str.strip(p.cDeclaration) for p in command.params)

# Comma separate the names of the parameters. ex) foo, bar, baz
def command_param_usage_text(command : Command):
    return ', '.join(p.name for p in command.params)


# Returns the category for the command based on its first parameter type
def get_command_category(command: Command):
    if not command.params:
        return 'CPUTimingCategory::Other'
    
    first_param_type = command.params[0].type
    if first_param_type == 'VkDevice':
        return 'CPUTimingCategory::VkDevice'
    elif first_param_type == 'VkPhysicalDevice':
        return 'CPUTimingCategory::VkPhysicalDevice'
    elif first_param_type == 'VkInstance':
        return 'CPUTimingCategory::VkInstance'
    elif first_param_type == 'VkQueue':
        return 'CPUTimingCategory::VkQueue'
    elif first_param_type == 'VkCommandBuffer':
        return 'CPUTimingCategory::VkCommandBuffer'
    else:
        return 'CPUTimingCategory::Other'


class CpuTimingGenerator(BaseGenerator):
    def __init__(self):
        BaseGenerator.__init__(self)

    def generate(self):
        self.generate_copyright()
        if self.filename == 'cputiming_dispatch.h':
            self.generate_dispatch_codegen()
        elif self.filename == 'cputiming_implementation.h':
            self.generate_implementation()


    def generate_copyright(self):
        self.write('''
/* Copyright (c) 2015-2026 Valve Corporation
 * Copyright (c) 2015-2026 LunarG, Inc.
 * Copyright (c) 2015-2017, 2019, 2021 Google Inc.
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

/*
 * This file is generated from the Khronos Vulkan XML API Registry.
 */
''')

    def generate_dispatch_codegen(self):

        self.write('#include "cputiming_handwritten_functions.h"')
        self.write('\n#include "timer.h"')
        self.write('\n#include "cputiming.h"')

        self.write('''

            // Autogen instance functions
            ''')

        protect = PlatformGuardHelper()
        for command in [x for x in self.vk.commands.values() if x.instance]:
            if command.name in MANUALLY_IMPLEMENTED_COMMANDS:
                continue
            protect.add_guard(self, command.protect)
            self.write(f'VKAPI_ATTR {command.returnType} VKAPI_CALL {command.name}({command_param_declaration_text(command)})')
            self.write('{')
            self.write(f'Timer timer({get_command_category(command)}, "{command.name}");')

            if command.name == 'vkGetPhysicalDeviceToolPropertiesEXT':
                self.write('''
                    static const VkPhysicalDeviceToolPropertiesEXT cputiming_layer_tool_props = {
                        VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TOOL_PROPERTIES_EXT,
                        nullptr,
                        "CPUTiming",
                        "1",
                        VK_TOOL_PURPOSE_PROFILING_BIT_EXT | VK_TOOL_PURPOSE_TRACING_BIT_EXT,
                        "The CPUTiming utility layer measures the timing of API calls.",
                        "CPUTiming"};

                    auto original_pToolProperties = pToolProperties;
                    if (pToolProperties != nullptr) {
                        *pToolProperties = cputiming_layer_tool_props;
                        pToolProperties = ((*pToolCount > 1) ? &pToolProperties[1] : nullptr);
                        (*pToolCount)--;
                    }''')
            if command.name == 'vkDestroyInstance':
                self.write('auto dispatch_key = get_dispatch_key(instance);')

            return_str = f'{command.returnType} result = ' if command.returnType != 'void' else ''
            self.write(f'{return_str}instance_dispatch_table({command.params[0].name})->{command.name[2:]}({command_param_usage_text(command)});')

            if command.name == 'vkEnumeratePhysicalDevices':
                self.write('''
                    if ((result == VK_SUCCESS || result == VK_INCOMPLETE) && pPhysicalDeviceCount != nullptr && pPhysicalDevices != nullptr) {
                        for (uint32_t i = 0; i < *pPhysicalDeviceCount; i++) {
                            CpuTiming::Get().SetVkInstance(pPhysicalDevices[i], instance);
                        }
                    }''')

            if command.name == 'vkDestroyInstance':
                self.write('destroy_instance_dispatch_table(dispatch_key);')

            if command.name == 'vkGetPhysicalDeviceToolPropertiesEXT':
                self.write('if (original_pToolProperties != nullptr) {')
                self.write('    pToolProperties = original_pToolProperties;')
                self.write('}\n')
                self.write('(*pToolCount)++;')


            if command.returnType != 'void':
                self.write('return result;')
            self.write('}')
        protect.add_guard(self, None)

        self.write('\n// Autogen device functions\n')

        for command in [x for x in self.vk.commands.values() if x.device and x.name not in MANUALLY_IMPLEMENTED_COMMANDS]:
            if command.name in MANUALLY_IMPLEMENTED_COMMANDS: # Double check, although list comprehension filters most
                continue

            protect.add_guard(self, command.protect)

            self.write(f'VKAPI_ATTR {command.returnType} VKAPI_CALL {command.name}({command_param_declaration_text(command)})')
            self.write('{')

            self.write(f'Timer timer({get_command_category(command)}, "{command.name}");')

            return_str = f'{command.returnType} result = ' if command.returnType != 'void' else ''
            self.write(f'{return_str}device_dispatch_table({command.params[0].name})->{command.name[2:]}({command_param_usage_text(command)});')

            if command.name == 'vkDestroyDevice':
                self.write('destroy_device_dispatch_table(get_dispatch_key(device));')

            if command.returnType != 'void':
                self.write('return result;')
            self.write('}')
        protect.add_guard(self, None)

        self.write('VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL cputiming_known_instance_functions(VkInstance instance, const char* pName)')
        self.write('{\n')
        for command in self.vk.commands.values():
            if command.name in ['vkEnumerateDeviceExtensionProperties', 'vkEnumerateInstanceVersion'] or (command.device and 'VK_EXT_debug_utils' not in command.extensions):
                continue
            protect.add_guard(self, command.protect)
            self.write(f'if(strcmp(pName, "{command.name}") == 0)')
            self.write(f'return reinterpret_cast<PFN_vkVoidFunction>({command.name});')
        protect.add_guard(self, None)
        self.write('\n    return nullptr;')
        self.write('}')

        self.write('VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL cputiming_known_device_functions(VkDevice device, const char* pName)')
        self.write('{\n')
        for command in [x for x in self.vk.commands.values() if x.device ]:
            protect.add_guard(self, command.protect)
            self.write(f'if(strcmp(pName, "{command.name}") == 0 && (!device || device_dispatch_table(device)->{command.name[2:]}))')
            self.write(f'return reinterpret_cast<PFN_vkVoidFunction>({command.name});')
        protect.add_guard(self, None)

        self.write('\n    return nullptr;')
        self.write('}')

    def generate_implementation(self):
        self.write('#pragma once\n')
        self.write('// Empty implementation for CPUTiming\n')
