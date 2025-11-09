#include "vk_core.h"
#include <hilog/log.h>

int vk_core_init(int32_t w, int32_t h, OHNativeWindow *win)
{
    OH_LOG_INFO(LOG_APP, "vk_core_init %dx%d", w, h);
    return 0;
}
void vk_core_draw(uint32_t count, const float *array)
{
    OH_LOG_INFO(LOG_APP, "vk_core_draw %u cards", count);
}
void vk_core_destroy(void)
{
    OH_LOG_INFO(LOG_APP, "vk_core_destroy");
}