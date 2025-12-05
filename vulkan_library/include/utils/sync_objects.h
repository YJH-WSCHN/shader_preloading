//Made by Han_feng

#ifndef VULKAN_LIBRARY_SYNC_OBJECTS_H
#define VULKAN_LIBRARY_SYNC_OBJECTS_H

#include "vulkan/vulkan.h"
#include "vector"

namespace vulkan {
    struct Sync_objects {
        std::vector<VkSemaphore> image_available_semaphores;
        std::vector<VkSemaphore> render_finished_semaphores;
        std::vector<VkFence> in_flight_fences;

        bool create(VkDevice device, int image_count);
        void destroy(VkDevice device);
    };
} // vulkan

#endif //VULKAN_LIBRARY_SYNC_OBJECTS_H