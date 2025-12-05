//Made by Han_feng

#include "utils/utils.h"
#include "utils/pipelines.h"
#include "utils/shaders/vertex_shader.h"
#include "utils/shaders/fragment_shader.h"

namespace vulkan {
    bool Pipelines::create(VkDevice device, const Render_pass *render_pass) {
        //Shaders
        auto vertex_shader_module = create_shader_module(device, vertex_shader, vertex_shader_len);
        auto fragment_shader_module = create_shader_module(device, fragment_shader, fragment_shader_len);

        std::vector<VkPipelineShaderStageCreateInfo> shader_stages(2);
        shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shader_stages[0].module = vertex_shader_module;
        shader_stages[0].pName = "main";

        shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shader_stages[1].module = fragment_shader_module;
        shader_stages[1].pName = "main";

        //Dynamic states
        std::vector dynamic_states = {
            VK_DYNAMIC_STATE_VIEWPORT,
            VK_DYNAMIC_STATE_SCISSOR,
        };

        VkPipelineDynamicStateCreateInfo dynamic_state_create_info = {};
        dynamic_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state_create_info.dynamicStateCount = dynamic_states.size();
        dynamic_state_create_info.pDynamicStates = dynamic_states.data();

        //Vertex input
        VkPipelineVertexInputStateCreateInfo vertex_input_state_create_info = {};
        vertex_input_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_state_create_info.vertexBindingDescriptionCount = 0;
        vertex_input_state_create_info.vertexAttributeDescriptionCount = 0;

        //Input assembly
        VkPipelineInputAssemblyStateCreateInfo input_assembly_create_info = {};
        input_assembly_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly_create_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly_create_info.primitiveRestartEnable = VK_FALSE;

        //Viewport state
        VkPipelineViewportStateCreateInfo viewport_state_create_info = {};
        viewport_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state_create_info.viewportCount = 1;
        viewport_state_create_info.scissorCount = 1;

        //Rasterizer
        VkPipelineRasterizationStateCreateInfo rasterization_state_create_info = {};
        rasterization_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state_create_info.depthClampEnable = VK_FALSE;
        rasterization_state_create_info.rasterizerDiscardEnable = VK_FALSE;
        rasterization_state_create_info.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state_create_info.lineWidth = 1.0;
        rasterization_state_create_info.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization_state_create_info.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rasterization_state_create_info.depthBiasEnable = VK_FALSE;

        //Multisampling
        VkPipelineMultisampleStateCreateInfo multisample_state_create_info = {};
        multisample_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state_create_info.sampleShadingEnable = VK_FALSE;
        multisample_state_create_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        //Depth stencil
        //TODO...

        //Color blend
        VkPipelineColorBlendAttachmentState color_blend_attachment_state = {};
        color_blend_attachment_state.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        color_blend_attachment_state.blendEnable = VK_TRUE;
        color_blend_attachment_state.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        color_blend_attachment_state.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        color_blend_attachment_state.colorBlendOp = VK_BLEND_OP_ADD;
        color_blend_attachment_state.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        color_blend_attachment_state.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        color_blend_attachment_state.alphaBlendOp = VK_BLEND_OP_ADD;

        VkPipelineColorBlendStateCreateInfo color_blend_state_create_info = {};
        color_blend_state_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        color_blend_state_create_info.logicOpEnable = VK_FALSE;
        color_blend_state_create_info.attachmentCount = 1;
        color_blend_state_create_info.pAttachments = &color_blend_attachment_state;

        //Layout
        VkPipelineLayoutCreateInfo layout_create_info = {};
        layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layout_create_info.setLayoutCount = 0;
        layout_create_info.pushConstantRangeCount = 0;

        if (vkCreatePipelineLayout(device, &layout_create_info, nullptr, &pipeline_layout) != VK_SUCCESS) {
            print_log(Error, "Failed to create pipeline layout!");
            return false;
        }

        //Graphics Pipelines
        VkGraphicsPipelineCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.stageCount = shader_stages.size();
        create_info.pStages = shader_stages.data();
        create_info.pVertexInputState = &vertex_input_state_create_info;
        create_info.pInputAssemblyState = &input_assembly_create_info;
        create_info.pViewportState = &viewport_state_create_info;
        create_info.pRasterizationState = &rasterization_state_create_info;
        create_info.pMultisampleState = &multisample_state_create_info;
        create_info.pDepthStencilState = nullptr;
        create_info.pColorBlendState = &color_blend_state_create_info;
        create_info.pDynamicState = &dynamic_state_create_info;
        create_info.layout = pipeline_layout;
        create_info.renderPass = render_pass->render_pass;
        create_info.subpass = 0;
        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;

        graphics_pipelines.resize(1);
        if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &create_info, nullptr, graphics_pipelines.data()) != VK_SUCCESS) {
            print_log(Error, "Failed to create graphics pipelines!");
            return false;
        }

        vkDestroyShaderModule(device, vertex_shader_module, nullptr);
        vkDestroyShaderModule(device, fragment_shader_module, nullptr);

        return true;
    }

    void Pipelines::destroy(VkDevice device) {
        vkDestroyPipelineLayout(device, pipeline_layout, nullptr);
        for (auto pipeline: graphics_pipelines) {
            vkDestroyPipeline(device, pipeline, nullptr);
        }
    }

    const VkPipeline& Pipelines::operator[](int index) const {
        return graphics_pipelines[index];
    }

    VkShaderModule Pipelines::create_shader_module(VkDevice device, unsigned char *shader_code, const size_t shader_code_size) {
        VkShaderModuleCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = shader_code_size;
        create_info.pCode = reinterpret_cast<uint32_t*>(shader_code);

        VkShaderModule shader_module;
        if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
            print_log(Error, "Failed to create shader module!");
        }

        return shader_module;
    }

} // vulkan