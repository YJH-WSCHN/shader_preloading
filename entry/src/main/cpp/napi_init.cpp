#include "napi/native_api.h"
#include "rust_vulkan_lib.h"
#include <native_window/external_window.h>
#include <unordered_map>

#include "hilog/log.h"

#define LOG_TAG "VK_NAPI"
#define LOGI(fmt, ...) \
    OH_LOG_Print(LOG_APP, LOG_INFO, 0, LOG_TAG, fmt, ##__VA_ARGS__)
#define LOGE(fmt, ...) \
    OH_LOG_Print(LOG_APP, LOG_ERROR, 0, LOG_TAG, fmt, ##__VA_ARGS__)

static std::unordered_map<int64_t, OHNativeWindow*> g_windowMap;
static int64_t ParseId(napi_env env, napi_value arg) {
    int64_t id = 0;
    bool lossless = true;
    if (napi_get_value_bigint_int64(env, arg, &id, &lossless) != napi_ok)
        return -1;
    return id;
}
static napi_value Add(napi_env env, napi_callback_info info)
{
    size_t argc = 2;
    napi_value args[2] = {nullptr};

    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    napi_valuetype valuetype0;
    napi_typeof(env, args[0], &valuetype0);

    napi_valuetype valuetype1;
    napi_typeof(env, args[1], &valuetype1);

    double value0;
    napi_get_value_double(env, args[0], &value0);

    double value1;
    napi_get_value_double(env, args[1], &value1);

    napi_value sum;
    napi_create_double(env, value0 + value1, &sum);

    return sum;

}
static inline Vulkan_application* toApp(int64_t v) {
    return reinterpret_cast<Vulkan_application*>(static_cast<uintptr_t>(v));
}
static inline int64_t fromApp(Vulkan_application* p) {
    return static_cast<int64_t>(reinterpret_cast<uintptr_t>(p));
}

static napi_value LogInit(napi_env env, napi_callback_info info) {
    size_t argc = 2;
    napi_value args[2];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    char path[512];
    size_t len;
    napi_get_value_string_utf8(env, args[0], path, sizeof(path), &len);
    uint32_t lvl;
    napi_get_value_uint32(env, args[1], &lvl);
    log_init(path, lvl);
    return nullptr;
}

static napi_value GetVulkanApplication(napi_env env, napi_callback_info info) {
    size_t argc = 4;
    napi_value args[4] = {nullptr};
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);

    /* 1. 取 surfaceId (bigint) */
    int64_t surfaceId = 0;
    bool lossless = true;
    if (napi_get_value_bigint_int64(env, args[0], &surfaceId, &lossless) != napi_ok) {
        LOGE("surfaceId 不是 bigint");
        napi_throw_type_error(env, nullptr, "surfaceId must be bigint");
        return nullptr;
    }
    LOGI("surfaceId = %{public}lld", surfaceId);
    
    OHNativeWindow* nativeWindow = reinterpret_cast<OHNativeWindow*>(surfaceId);
    if (!nativeWindow) {
        LOGE("nativeWindow 是 nullptr");
        napi_throw_error(env, nullptr, "invalid surface pointer");
        return nullptr;
    }
    LOGI("nativeWindow = %{public}p", nativeWindow);
    
    uint32_t width = 0, height = 0;
    napi_get_value_uint32(env, args[1], &width);
    napi_get_value_uint32(env, args[2], &height);
    if (width == 0 || height == 0) {
        LOGE("width 或 height 为 0");
        napi_throw_error(env, nullptr, "width/height is 0");
        return nullptr;
    }
    LOGI("width = %{public}u, height = %{public}u", width, height);
    
    char vkPath[256] = {0};
    size_t len = 0;
    napi_get_value_string_utf8(env, args[3], vkPath, sizeof(vkPath), &len);
    LOGI("vulkanPath = %{public}s", vkPath[0] ? vkPath : "<empty>");
    
    Vulkan_application* app = nullptr;
    Status_code sc = get_vulkan_application(nativeWindow, width, height, vkPath, &app);
    if (sc != Success || !app) {
        LOGE("Rust 创建失败, status=%{public}d", sc);
        napi_value null;
        napi_get_null(env, &null);
        return null;
    }
    LOGI("Rust 创建成功, app=%p", app);

    napi_value ret;
    napi_create_bigint_uint64(env, reinterpret_cast<uint64_t>(app), &ret);
    return ret;
}

static napi_value DrawFrame(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t v;
    napi_get_value_int64(env, args[0], &v);
    Status_code sc = draw_frame(toApp(v));
    napi_value ret;
    napi_create_int32(env, sc, &ret);
    return ret;
}

static napi_value DestroyVulkanApplication(napi_env env, napi_callback_info info) {
    size_t argc = 1;
    napi_value args[1];
    napi_get_cb_info(env, info, &argc, args, nullptr, nullptr);
    int64_t v;
    napi_get_value_int64(env, args[0], &v);
    destroy_vulkan_application(toApp(v));
    return nullptr;
}
EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        { "add", nullptr, Add, nullptr, nullptr, nullptr, napi_default, nullptr },
        {"logInit", nullptr, LogInit, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"getVulkanApplication", nullptr, GetVulkanApplication, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"drawFrame", nullptr, DrawFrame, nullptr, nullptr, nullptr, napi_default, nullptr},
        {"destroyVulkanApplication", nullptr, DestroyVulkanApplication, nullptr, nullptr, nullptr, napi_default, nullptr}
    };
    napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc);
    return exports;
}
EXTERN_C_END

static napi_module demoModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = Init,
    .nm_modname = "entry",
    .nm_priv = ((void*)0),
    .reserved = { 0 },
};

extern "C" __attribute__((constructor)) void RegisterEntryModule(void)
{
    napi_module_register(&demoModule);
}
