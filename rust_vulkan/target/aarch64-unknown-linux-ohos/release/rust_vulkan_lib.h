//Made by Han_feng

#ifndef RUST_VULKAN_LIB_H
#define RUST_VULKAN_LIB_H

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef enum Status_code {
  Success = 0,
  Failure = 1,
} Status_code;

typedef struct Logger Logger;

typedef struct Vulkan_application Vulkan_application;

enum Status_code log_init(const char *log_file, unsigned long long log_level);

enum Status_code get_logger(unsigned long long log_level,
                            unsigned long long logger_size,
                            struct Logger **logger);

enum Status_code get_last_message_length(const struct Logger *logger, unsigned long long *length);

enum Status_code get_last_message(struct Logger *logger, char *data);

void destroy_logger(struct Logger *logger);

enum Status_code get_vulkan_application(void *window_handle,
                                        unsigned int width,
                                        unsigned int height,
                                        const char *vulkan_path,
                                        struct Vulkan_application **vulkan_application);

enum Status_code draw_frame(struct Vulkan_application *vulkan_application);

void destroy_vulkan_application(struct Vulkan_application *vulkan_application);

#endif  /* RUST_VULKAN_LIB_H */
