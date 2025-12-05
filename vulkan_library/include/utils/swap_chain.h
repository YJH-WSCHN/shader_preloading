//Made by Han_feng

#ifndef VULKAN_LIBRARY_SWAP_CHAIN_H
#define VULKAN_LIBRARY_SWAP_CHAIN_H

#include "utils/render_pass.h"
#include "utils/queues.h"
#include "vulkan/vulkan.h"

#include "vector"

namespace vulkan {
    struct Window;

    struct Swap_chain_supports {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;

        Swap_chain_supports(VkPhysicalDevice physical_device, VkSurfaceKHR surface);
        bool check_validation() const;
        VkSurfaceFormatKHR choose_format() const;
        VkPresentModeKHR choose_present_mode() const;
        VkExtent2D get_extent(unsigned int width, unsigned int height) const;
    };

    struct Swap_chain {
        VkSwapchainKHR swap_chain;
        std::vector<VkImage> images;
        std::vector<VkImageView> image_views;
        std::vector<VkFramebuffer> frame_buffers;
        VkFormat image_format;
        VkExtent2D extent;

        bool create(VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface, const Window *window, const Queues *queues, VkSwapchainKHR old_swap_chain = nullptr);
        void destroy(VkDevice device);

        unsigned int get_image_count() const;
        void clean_up_images(VkDevice device);
        bool update_images(VkDevice device, const Render_pass *render_pass);
    };
} // vulkan

#endif //VULKAN_LIBRARY_SWAP_CHAIN_H