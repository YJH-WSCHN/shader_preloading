//Made by Han_feng

#include "config.h"
#include "application.h"
#include "utils/utils.h"
#include "utils/swap_chain.h"

#include "set"
#include "string"

namespace vulkan {
    #ifdef WINDOWS_TEST
    std::vector<const char*> Application::INSTANCE_EXTENSIONS = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_KHR_WIN32_SURFACE_EXTENSION_NAME
    };
    #elif defined(VULKAN_DEBUG)
    std::vector<const char*> Application::INSTANCE_EXTENSIONS = {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_OHOS_SURFACE_EXTENSION_NAME
    };
    #else
    std::vector<const char*> Application::INSTANCE_EXTENSIONS = {
        VK_KHR_SURFACE_EXTENSION_NAME,
        VK_OHOS_SURFACE_EXTENSION_NAME
    };
    #endif

    std::vector<const char*> Application::DEVICE_EXTENSIONS = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    #ifdef VULKAN_DEBUG
    const std::vector VALIDATION_LAYERS = {
        "VK_LAYER_KHRONOS_validation"
    };

    static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
        VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
        VkDebugUtilsMessageTypeFlagsEXT message_type,
        const VkDebugUtilsMessengerCallbackDataEXT* p_callback_data,
        void* user_data
    ) {
        Log_level level = Unknown;
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT){
            level = static_cast<Log_level>(level | Info);
        }
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
            level = static_cast<Log_level>(level | Verbose);
        }
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
            level = static_cast<Log_level>(level | Warning);
        }
        if (message_severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
            level = static_cast<Log_level>(level | Error);
        }

        print_log(level, p_callback_data->pMessage);

        return VK_FALSE;
    }

    VkDebugUtilsMessengerCreateInfoEXT populate_debug_messenger_create_info() {
        VkDebugUtilsMessengerCreateInfoEXT create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        create_info.pfnUserCallback = debug_callback;
        create_info.pUserData = nullptr;

        return create_info;
    }
    #endif

    //Vulkan application
    Application::Application(const Window *window): window(window) {
        create_instance();
        create_debug_messenger();
        create_surface(window->window);
        get_physical_device();
        queues.get_indices(physical_device, surface);
        create_device();
        queues.get_queues(device);

        valid &= swap_chain.create(physical_device, device, surface, window, &queues);
        valid &= render_pass.create(device, swap_chain.image_format);
        valid &= swap_chain.update_images(device, &render_pass);
        valid &= pipelines.create(device, &render_pass);
        valid &= command_context.create(device, &queues);
        valid &= sync_objects.create(device, swap_chain.get_image_count());
    }

    Application::~Application() {
        vkDeviceWaitIdle(device);

        sync_objects.destroy(device);
        command_context.destroy(device);
        pipelines.destroy(device);
        swap_chain.destroy(device);
        render_pass.destroy(device);
        vkDestroyDevice(device, nullptr);
        vkDestroySurfaceKHR(instance, surface, nullptr);

        #ifdef VULKAN_DEBUG
        if (const auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")); func != nullptr) {
            func(instance, debug_messenger, nullptr);
        }
        #endif

        vkDestroyInstance(instance, nullptr);
    }

    bool Application::validate() const {
        return valid;
    }

    std::optional<bool> Application::draw_frame() {
        bool out_of_date = false;

        vkWaitForFences(device, 1, &sync_objects.in_flight_fences[current_frame], VK_TRUE, UINT64_MAX);

        unsigned int image_index;
        switch (vkAcquireNextImageKHR(device, swap_chain.swap_chain, UINT64_MAX, sync_objects.image_available_semaphores[current_frame], VK_NULL_HANDLE, &image_index)) {
            case VK_SUCCESS:
                break;
            case VK_SUBOPTIMAL_KHR:
                out_of_date = true;
                break;
            case VK_ERROR_OUT_OF_DATE_KHR:
                return true;
            default:
                print_log(Error, "Failed to acquire swap chain image");
                return std::nullopt;
        }

        vkResetFences(device, 1, &sync_objects.in_flight_fences[current_frame]);

        auto draw_buffer = command_context.get_draw_buffer(current_frame, image_index, &swap_chain, &render_pass, &pipelines);

        if (!draw_buffer) {
            print_log(Error, "Failed to acquire draw buffer");
            return std::nullopt;
        }

        VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = 1;
        submit_info.pWaitSemaphores = &sync_objects.image_available_semaphores[current_frame];
        submit_info.pWaitDstStageMask = wait_stages;
        submit_info.signalSemaphoreCount = 1;
        submit_info.pSignalSemaphores = &sync_objects.render_finished_semaphores[image_index];
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &draw_buffer;

        if (vkQueueSubmit(queues.graphics_queue, 1, &submit_info, sync_objects.in_flight_fences[current_frame]) != VK_SUCCESS) {
            print_log(Error, "Failed to submit draw command buffer!");
            return std::nullopt;
        }

        VkPresentInfoKHR present_info = {};
        present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        present_info.waitSemaphoreCount = 1;
        present_info.pWaitSemaphores = &sync_objects.render_finished_semaphores[image_index];
        present_info.swapchainCount = 1;
        present_info.pSwapchains = &swap_chain.swap_chain;
        present_info.pImageIndices = &image_index;

        switch (vkQueuePresentKHR(queues.present_queue, &present_info)) {
            case VK_SUCCESS:
                break;
            case VK_SUBOPTIMAL_KHR: case VK_ERROR_OUT_OF_DATE_KHR:
                out_of_date = true;
                break;
            default:
                print_log(Error, "Failed to present swap chain image!");
                return std::nullopt;
        }

        current_frame = (current_frame+1)%MAX_FRAMES_IN_FLIGHT;

        return out_of_date;
    }

    bool Application::refresh() {
        auto result = true;

        //Swap chain recreate
        auto old_swap_chain = std::move(swap_chain);
        result &= swap_chain.create(physical_device, device, surface, window, &queues, old_swap_chain.swap_chain);
        result &= swap_chain.update_images(device, &render_pass);

        vkQueueWaitIdle(queues.present_queue);
        old_swap_chain.destroy(device);

        return result;
    }

    void Application::create_instance() {
        VkApplicationInfo app_info = {};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "OpenHarmony window";
        app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.pEngineName = "Hanfeng's Engine";
        app_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        app_info.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;
        create_info.enabledExtensionCount = INSTANCE_EXTENSIONS.size();
        create_info.ppEnabledExtensionNames = INSTANCE_EXTENSIONS.data();

        #ifdef VULKAN_DEBUG
        create_info.enabledLayerCount = VALIDATION_LAYERS.size();
        create_info.ppEnabledLayerNames = VALIDATION_LAYERS.data();

        auto debug_create_info = populate_debug_messenger_create_info();
        create_info.pNext = &debug_create_info;
        #endif

        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
            print_log(Error, "Failed to create instance!");
            valid = false;
        }
    }

    void Application::create_debug_messenger() {
        #ifdef VULKAN_DEBUG
        auto create_info = populate_debug_messenger_create_info();
        if (const auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")); func != nullptr) {
            if (func(instance, &create_info, nullptr, &debug_messenger) != VK_SUCCESS) {
                print_log(Error, "Failed to create debug messenger!");
                valid = false;
            }
        }
        else {
            print_log(Error, "Failed to get vkCreateDebugUtilsMessengerEXT!");
            valid = false;
        }
        #endif
    }

    void Application::create_surface(void *window_handle) {
        #ifdef WINDOWS_TEST
        VkWin32SurfaceCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
        create_info.hwnd = static_cast<HWND>(window_handle);
        create_info.hinstance = GetModuleHandle(nullptr);

        if (vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface) != VK_SUCCESS) {
            print_log(Error, "Failed to create surface!");
            valid = false;
        }
        #else
        VkOHSurfaceCreateInfoOHOS create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SURFACE_CREATE_INFO_OHOS;
        create_info.window = static_cast<OHNativeWindow *>(window_handle);

        if (VkCreateSurfaceOHOS(instance, &create_info, nullptr, &surface) != VK_SUCCESS) {
            print_log(Error, "Failed to create surface!");
            valid = false;
        }
        else {
            print_log(Info, "Successfully created surface!");
        }
        #endif
    }


    void Application::get_physical_device() {
        unsigned int device_count = 0;
        vkEnumeratePhysicalDevices(instance, &device_count, nullptr);
        if (device_count == 0) {
            print_log(Error, "Failed to find GPUs with Vulkan support!");
            valid = false;
        }

        std::vector<VkPhysicalDevice> physical_devices(device_count);
        vkEnumeratePhysicalDevices(instance, &device_count, physical_devices.data());

        int score = 0;
        for (const auto& alternative_device: physical_devices) {
            if (const int current_score = get_physical_device_score(alternative_device, surface); current_score > score) {
                score = current_score;
                physical_device = alternative_device;
            }
        }

        if (physical_device == nullptr) {
            print_log(Error, "Failed to get a suitable physical device!");
            valid = false;
        }
        #ifdef VULKAN_DEBUG
        else {
            VkPhysicalDeviceProperties device_properties;
            vkGetPhysicalDeviceProperties(physical_device, &device_properties);
            print_log(Verbose, "Choose physical device: %s", device_properties.deviceName);
        }
        #endif
    }

    void Application::create_device() {
        auto queue_create_infos = queues.get_create_infos();

        VkPhysicalDeviceFeatures device_features = {};

        VkDeviceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        create_info.queueCreateInfoCount = queue_create_infos.size();
        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.enabledExtensionCount = DEVICE_EXTENSIONS.size();
        create_info.ppEnabledExtensionNames = DEVICE_EXTENSIONS.data();
        create_info.pEnabledFeatures = &device_features;

        if (vkCreateDevice(physical_device, &create_info, nullptr, &device) != VK_SUCCESS) {
            print_log(Error, "Failed to create device!");
            valid = false;
        }
    }


    int Application::get_physical_device_score(VkPhysicalDevice target, VkSurfaceKHR surface) {
        VkPhysicalDeviceProperties properties;
        VkPhysicalDeviceFeatures features;
        vkGetPhysicalDeviceProperties(target, &properties);
        vkGetPhysicalDeviceFeatures(target, &features);

        unsigned int extension_count = 0;
        vkEnumerateDeviceExtensionProperties(target, nullptr, &extension_count, nullptr);
        std::vector<VkExtensionProperties> available_extensions(extension_count);
        vkEnumerateDeviceExtensionProperties(target, nullptr, &extension_count, available_extensions.data());

        std::set<std::string> required_extensions(DEVICE_EXTENSIONS.begin(), DEVICE_EXTENSIONS.end());

        for (const auto& [extensionName, _]: available_extensions) {
            required_extensions.erase(extensionName);
        }

        if (!required_extensions.empty()) {
            return -1;
        }

        const auto swap_chain_support = Swap_chain_supports(target, surface);
        if (!swap_chain_support.check_validation()) {
            return -1;
        }

        switch (properties.deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return 1919810;
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return 114514;
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return 24;
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return 3;
            default:
                return 1;
        }
    }
} // vulkan