//Made by Han_feng

#ifndef VULKAN_LIBRARY_CONFIG_H
#define VULKAN_LIBRARY_CONFIG_H

#pragma once

#ifdef _WIN32
    #define WINDOWS_TEST
    #define VULKAN_DEBUG
    #define VK_USE_PLATFORM_WIN32_KHR

    #define GLFW_INCLUDE_VULKAN
    #include "GLFW/glfw3.h"
    #define GLFW_EXPOSE_NATIVE_WIN32
    #include "GLFW/glfw3native.h"
#else
    //Todo: Include OHOS libraries here...
    #include "vulkan/vulkan_ohos.h"
    //#include "hilog/log.h"

    #ifdef NDEBUG
        #define VULKAN_RELEASE
    #else
        #define VULKAN_DEBUG
    #endif
#endif

#endif //VULKAN_LIBRARY_CONFIG_H