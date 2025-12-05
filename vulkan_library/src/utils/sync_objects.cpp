//Made by Han_feng

#include "utils/sync_objects.h"
#include "utils/utils.h"

namespace vulkan {
    bool Sync_objects::create(VkDevice device, int image_count) {
        VkSemaphoreCreateInfo semaphore_info = {};
        semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fence_info = {};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        image_available_semaphores.resize(MAX_FRAMES_IN_FLIGHT);
        render_finished_semaphores.resize(image_count);
        in_flight_fences.resize(MAX_FRAMES_IN_FLIGHT);

        for (int i=0;i<MAX_FRAMES_IN_FLIGHT;i++) {
            if (vkCreateSemaphore(device, &semaphore_info, nullptr, &image_available_semaphores[i]) != VK_SUCCESS) {
                print_log(Error, "Failed to create image available semaphore!");
                return false;
            }
        }

        for (int i=0;i<image_count;i++) {
            if (vkCreateSemaphore(device, &semaphore_info, nullptr, &render_finished_semaphores[i]) != VK_SUCCESS) {
                print_log(Error, "Failed to create render finished semaphore!");
                return false;
            }
        }

        for (int i=0;i<MAX_FRAMES_IN_FLIGHT;i++) {
            if (vkCreateFence(device, &fence_info, nullptr, &in_flight_fences[i]) != VK_SUCCESS) {
                print_log(Error, "Failed to create in flight fence!");
                return false;
            }
        }

        return true;
    }

    void Sync_objects::destroy(VkDevice device) {
        for (auto semaphore: image_available_semaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        for (auto semaphore: render_finished_semaphores) {
            vkDestroySemaphore(device, semaphore, nullptr);
        }
        for (auto fence: in_flight_fences) {
            vkDestroyFence(device, fence, nullptr);
        }
    }
} // vulkan