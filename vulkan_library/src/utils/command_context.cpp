//Made by Han_feng

#include "utils/command_context.h"
#include "utils/utils.h"

namespace vulkan {
    bool Command_context::create(VkDevice device, const Queues *queues) {
        VkCommandPoolCreateInfo pool_create_info = {};
        pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        pool_create_info.queueFamilyIndex = queues->graphics_family.value();

        if (vkCreateCommandPool(device, &pool_create_info, nullptr, &command_pool) != VK_SUCCESS) {
            print_log(Error, "Failed to create command pool!");
        }

        VkCommandBufferAllocateInfo buffer_allocate_info = {};
        buffer_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        buffer_allocate_info.commandPool = command_pool;
        buffer_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        buffer_allocate_info.commandBufferCount = MAX_FRAMES_IN_FLIGHT;

        draw_buffers.resize(MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateCommandBuffers(device, &buffer_allocate_info, draw_buffers.data()) != VK_SUCCESS) {
            print_log(Error, "Failed to allocate command buffers!");
            return false;
        }

        return true;
    }

    void Command_context::destroy(VkDevice device) {
        vkDestroyCommandPool(device, command_pool, nullptr);
    }

    VkCommandBuffer Command_context::get_draw_buffer(const int index, const int image_index, const Swap_chain *swap_chain, const Render_pass *render_pass, const Pipelines *pipelines) {
        //Render pass begin
        VkClearValue clear_value = {{{0.0, 0.0, 0.0, 1.0}}};
        VkRenderPassBeginInfo render_pass_info = {};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        render_pass_info.renderPass = render_pass->render_pass;
        render_pass_info.framebuffer = swap_chain->frame_buffers[image_index];
        render_pass_info.renderArea.offset = {0, 0};
        render_pass_info.renderArea.extent = swap_chain->extent;
        render_pass_info.clearValueCount = 1;
        render_pass_info.pClearValues = &clear_value;

        //Viewport state
        VkViewport viewport = {};
        viewport.x = 0.0;
        viewport.y = 0.0;
        viewport.width = static_cast<float>(swap_chain->extent.width);
        viewport.height = static_cast<float>(swap_chain->extent.height);
        viewport.minDepth = 0.0;
        viewport.maxDepth = 1.0;

        VkRect2D scissor = {};
        scissor.offset = {0, 0};
        scissor.extent = swap_chain->extent;

        //Begin
        auto buffer = draw_buffers[index];
        vkResetCommandBuffer(buffer, VK_COMMAND_BUFFER_LEVEL_PRIMARY);

        VkCommandBufferBeginInfo begin_info = {};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        begin_info.pInheritanceInfo = nullptr;

        if (vkBeginCommandBuffer(buffer, &begin_info) != VK_SUCCESS) {
            print_log(Error, "Failed to begin command buffer!");
            return nullptr;
        }

        vkCmdBeginRenderPass(buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);
        vkCmdBindPipeline(buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, (*pipelines)[0]);
        vkCmdSetViewport(buffer, 0, 1, &viewport);
        vkCmdSetScissor(buffer, 0, 1, &scissor);
        vkCmdDraw(buffer, 3, 1, 0, 0);
        vkCmdEndRenderPass(buffer);

        if (vkEndCommandBuffer(buffer) != VK_SUCCESS) {
            print_log(Error, "Failed to end command buffer!");
            return nullptr;
        }

        return buffer;
    }

} // vulkan