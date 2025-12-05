//Made by Han_feng

#include "utils/utils.h"
#include "utils/swap_chain.h"
#include "application.h"
#include "stdint.h"
#include "limits"
#include "algorithm"

namespace vulkan {
    Swap_chain_supports::Swap_chain_supports(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

        unsigned int format_count;
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, nullptr);
        formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &format_count, formats.data());

        unsigned int present_mode_count;
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);
        present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes.data());
    }

    bool Swap_chain_supports::check_validation() const {
        return !formats.empty() && !present_modes.empty();
    }

    VkSurfaceFormatKHR Swap_chain_supports::choose_format() const {
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR Swap_chain_supports::choose_present_mode() const {
        for (const auto& present_mode : present_modes) {
            if (present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return present_mode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D Swap_chain_supports::get_extent(unsigned int width, unsigned int height) const {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            return capabilities.currentExtent;
        }
        else {
            VkExtent2D extent{width, height};
            extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return extent;
        }
    }

    bool Swap_chain::create(VkPhysicalDevice physical_device, VkDevice device, VkSurfaceKHR surface, const Window *window, const Queues *queues, VkSwapchainKHR old_swap_chain) {
        auto supports = Swap_chain_supports(physical_device, surface);

        auto surface_format = supports.choose_format();
        auto present_mode = supports.choose_present_mode();
        auto surface_extent = supports.get_extent(window->width, window->height);

        unsigned int image_count = supports.capabilities.minImageCount + 2;
        if (supports.capabilities.maxImageCount > 0 && image_count > supports.capabilities.maxImageCount) {
            image_count = supports.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        create_info.surface = surface;
        create_info.minImageCount = image_count;
        create_info.imageFormat = surface_format.format;
        create_info.imageColorSpace = surface_format.colorSpace;
        create_info.imageExtent = surface_extent;
        create_info.imageArrayLayers = 1;
        create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t indices[] = {queues->graphics_family.value(), queues->present_family.value()};
        if (queues->graphics_family != queues->present_family) {
            create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = indices;
        }
        else {
            create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            create_info.queueFamilyIndexCount = 0;
            create_info.pQueueFamilyIndices = nullptr;
        }

        create_info.preTransform = supports.capabilities.currentTransform;
        create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        create_info.presentMode = present_mode;
        create_info.clipped = VK_TRUE;
        create_info.oldSwapchain = old_swap_chain;

        image_format = surface_format.format;
        extent = surface_extent;

        if (vkCreateSwapchainKHR(device, &create_info, nullptr, &swap_chain) != VK_SUCCESS) {
            print_log(Error, "Failed to create swap chain!");
            return false;
        }

        return true;
    }

    void Swap_chain::destroy(VkDevice device){
        clean_up_images(device);
        vkDestroySwapchainKHR(device, swap_chain, nullptr);
    }

    unsigned int Swap_chain::get_image_count() const {
        return images.size();
    }

    void Swap_chain::clean_up_images(VkDevice device){
        for (int i=0;i<image_views.size();i++) {
            vkDestroyImageView(device, image_views[i], nullptr);
            vkDestroyFramebuffer(device, frame_buffers[i], nullptr);
        }

        image_views.clear();
        frame_buffers.clear();
    }

    bool Swap_chain::update_images(VkDevice device, const Render_pass *render_pass) {
        clean_up_images(device);

        unsigned int image_count = 0;
        vkGetSwapchainImagesKHR(device, swap_chain, &image_count, nullptr);
        images.resize(image_count);
        vkGetSwapchainImagesKHR(device, swap_chain, &image_count, images.data());

        image_views.resize(image_count);
        frame_buffers.resize(image_count);
        for (int i=0;i<image_count;i++) {
            VkImageViewCreateInfo image_view_create_info{};
            image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            image_view_create_info.image = images[i];
            image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            image_view_create_info.format = image_format;
            image_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            image_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            image_view_create_info.subresourceRange.baseMipLevel = 0;
            image_view_create_info.subresourceRange.levelCount = 1;
            image_view_create_info.subresourceRange.baseArrayLayer = 0;
            image_view_create_info.subresourceRange.layerCount = 1;

            if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_views[i]) != VK_SUCCESS) {
                print_log(Error, "Failed to create image view!");
                return false;
            }

            VkFramebufferCreateInfo frame_buffer_create_info{};
            frame_buffer_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            frame_buffer_create_info.renderPass = render_pass->render_pass;
            frame_buffer_create_info.attachmentCount = 1;
            frame_buffer_create_info.pAttachments = &image_views[i];
            frame_buffer_create_info.width = extent.width;
            frame_buffer_create_info.height = extent.height;
            frame_buffer_create_info.layers = 1;

            if (vkCreateFramebuffer(device, &frame_buffer_create_info, nullptr, &frame_buffers[i]) != VK_SUCCESS) {
                print_log(Error, "Failed to create frame buffer!");
                return false;
            }
        }

        return true;
    }
} // vulkan