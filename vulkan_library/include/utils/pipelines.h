//Made by Han_feng

#ifndef VULKAN_LIBRARY_PIPELINES_H
#define VULKAN_LIBRARY_PIPELINES_H

#include "utils/render_pass.h"
#include "vulkan/vulkan.h"
#include "vector"

namespace vulkan {
    struct Pipelines {
        VkPipelineLayout pipeline_layout;
        std::vector<VkPipeline> graphics_pipelines;

        bool create(VkDevice device, const Render_pass *render_pass);
        void destroy(VkDevice device);

        const VkPipeline& operator[](int index) const;

        static VkShaderModule create_shader_module(VkDevice device, unsigned char* shader_code, size_t shader_code_size);
    };
} // vulkan

#endif //VULKAN_LIBRARY_PIPELINES_H