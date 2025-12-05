//Made by Han_feng

#ifndef VULKAN_LIBRARY_COMMAND_CONTEXT_H
#define VULKAN_LIBRARY_COMMAND_CONTEXT_H

#include "utils/pipelines.h"
#include "utils/queues.h"
#include "utils/render_pass.h"
#include "utils/swap_chain.h"
#include "vulkan/vulkan.h"

namespace vulkan {
    struct Command_context {
        VkCommandPool command_pool;
        std::vector<VkCommandBuffer> draw_buffers;

        bool create(VkDevice device, const Queues *queues);
        void destroy(VkDevice device);

        VkCommandBuffer get_draw_buffer(int index, int image_index, const Swap_chain *swap_chain, const Render_pass *render_pass, const Pipelines *pipelines);
    };
} // vulkan

#endif //VULKAN_LIBRARY_COMMAND_CONTEXT_H