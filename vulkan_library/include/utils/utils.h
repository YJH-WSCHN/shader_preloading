//
// Created by Hanfeng on 2025/12/2.
//

#ifndef VULKAN_LIBRARY_UTILS_H
#define VULKAN_LIBRARY_UTILS_H

#include "config.h"
#include "stdio.h"

namespace vulkan {
    enum Log_level {
        Unknown = 0,
        Info = 1,
        Verbose = 2,
        Warning = 4,
        Error = 8
    };

    inline void print_log(Log_level level, const char* message_format, ...) {
        va_list args;

        #ifdef WINDOWS_TEST
        const char* prefix = "Unknown";
        switch (level) {
            case Info:
                prefix = "Info";
                break;
            case Verbose:
                prefix = "Verbose";
                break;
            case Warning:
                prefix = "Warning";
                break;
            case Error:
                prefix = "Error";
                break;
            default:
                break;
        }
        printf("[%s]: ", prefix);
        va_start(args, message_format);
        vprintf(message_format, args);
        va_end(args);
        printf("\n");
        #elif defined(VULKAN_DEBUG)
        //TODO: Hilog here...
        #endif
    }

    inline constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;
}

#endif //VULKAN_LIBRARY_UTILS_H