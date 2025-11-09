#ifndef VK_CORE_H_
#define VK_CORE_H_

#include <stdint.h>
#include <native_window/external_window.h>


int  vk_core_init(int32_t width, int32_t height, OHNativeWindow* window);
void vk_core_draw(uint32_t card_count, const float *array);
void vk_core_destroy(void);
 
#endif // VK_CORE_H_