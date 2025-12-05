//
// Created by Hanfeng on 2025/12/2.
//

#ifndef VULKAN_LIBRARY_QUEUES_H
#define VULKAN_LIBRARY_QUEUES_H

#include "optional"
#include "vulkan/vulkan.h"
#include "vector"

namespace vulkan {
    struct Queues {
        std::optional<unsigned int> graphics_family;
        VkQueue graphics_queue;

        std::optional<unsigned int> present_family;
        VkQueue present_queue;

        std::vector<float> queue_priorities;

        Queues();
        void get_indices(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
        void get_queues(VkDevice device);
        bool check_complete() const;
        std::vector<VkDeviceQueueCreateInfo> get_create_infos() const;
    };
} // vulkan

#endif //VULKAN_LIBRARY_QUEUES_H