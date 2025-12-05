//Made by Han_feng

#ifndef VULKAN_LIBRARY_RENDER_PASS_H
#define VULKAN_LIBRARY_RENDER_PASS_H

#include "vulkan/vulkan.h"

namespace vulkan {
    struct Render_pass {
        VkRenderPass render_pass;

        bool create(VkDevice device, VkFormat swap_chain_image_format);
        void destroy(VkDevice device);
    };
}

#endif //VULKAN_LIBRARY_RENDER_PASS_H