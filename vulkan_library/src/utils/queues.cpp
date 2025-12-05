//
// Created by Hanfeng on 2025/12/2.
//

#include "utils/queues.h"
#include "vector"
#include "set"

namespace vulkan {
    Queues::Queues(): graphics_family(std::nullopt), graphics_queue(nullptr), present_family(std::nullopt), present_queue(nullptr) {
        queue_priorities = {1.0};
    }

    void Queues::get_indices(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        unsigned int queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

        std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
        vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());

        for (int i=0;i<queue_families.size();i++) {
            if (queue_families[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_family = i;
            }

            VkBool32 present_support = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
            if (present_support) {
                present_family = i;
            }

            if (check_complete()) {
                break;
            }
        }
    }

    void Queues::get_queues(VkDevice device) {
        vkGetDeviceQueue(device, graphics_family.value(), 0, &graphics_queue);
        vkGetDeviceQueue(device, present_family.value(), 0, &present_queue);
    }

    bool Queues::check_complete() const {
        return graphics_family.has_value() && present_family.has_value();
    }

    std::vector<VkDeviceQueueCreateInfo> Queues::get_create_infos() const {
        const std::set indices = {graphics_family.value(), present_family.value()};

        std::vector<VkDeviceQueueCreateInfo> create_infos;
        create_infos.reserve(indices.size());
        for (const auto i : indices) {
            VkDeviceQueueCreateInfo queue_create_info = {};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = i;
            queue_create_info.queueCount = queue_priorities.size();
            queue_create_info.pQueuePriorities = queue_priorities.data();
            create_infos.push_back(queue_create_info);
        }

        return create_infos;
    }

} // vulkan