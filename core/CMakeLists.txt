cmake_minimum_required(VERSION 3.16)
project(vulkancore)

find_package(Vulkan REQUIRED)

add_library(vulkancore SHARED          # 可改为 STATIC
    src/vk_core.c
    src/vk_pipeline.c
    src/vk_utils.c
)
target_include_directories(vulkancore PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(vulkancore PUBLIC Vulkan::Vulkan)
target_link_libraries(vulkancore PUBLIC libhilog_ndk.z.so)
# 安装规则（可选）
install(TARGETS vulkancore DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)