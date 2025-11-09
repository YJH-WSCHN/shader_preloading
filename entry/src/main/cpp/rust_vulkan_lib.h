//Made by Han_feng

#ifndef RUST_VULKAN_LIB_H
#define RUST_VULKAN_LIB_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum Status_code {
  Success = 0,
  Failure = 1,
} Status_code;

typedef struct Vulkan_application Vulkan_application;

void log_init(const char *log_file, unsigned int log_level);

enum Status_code get_vulkan_application(void *window_handle,
                                        unsigned int width,
                                        unsigned int height,
                                        const char *vulkan_path,
                                        struct Vulkan_application **vulkan_application);

enum Status_code draw_frame(struct Vulkan_application *vulkan_application);

void destroy_vulkan_application(struct Vulkan_application *vulkan_application);

#ifdef __cplusplus
}
#endif

#endif  /* RUST_VULKAN_LIB_H */