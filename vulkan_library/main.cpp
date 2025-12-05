//Made by Han_feng

#include "application.h"
#include "config.h"
#include "utils/utils.h"

using namespace vulkan;

int main() {
    #ifdef WINDOWS_TEST
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    int width = 800, height = 600;
    auto glfw_window = glfwCreateWindow(width, height, "Vulkan Windows Test", nullptr, nullptr);
    glfwGetWindowSize(glfw_window, &width, &height);

    auto window = Window{glfwGetWin32Window(glfw_window), static_cast<unsigned int>(width), static_cast<unsigned int>(height)};
    auto vulkan_app = Application(&window);

    if (!vulkan_app.validate()) {
        goto end;
    }

    while (!glfwWindowShouldClose(glfw_window)) {
        glfwPollEvents();
        auto result = vulkan_app.draw_frame();
        if (result.has_value()) {
            if (result.value()) {
                if (!vulkan_app.refresh()) {
                    goto err;
                }
            }
        }
        else {
            err:
            fprintf(stderr, "Fatal Error!");
            break;
        }
    }

    end:
    glfwDestroyWindow(glfw_window);
    glfwTerminate();
    #else
    print_log(Warning, "Please run this test on Windows!");
    #endif
    return 0;
}
