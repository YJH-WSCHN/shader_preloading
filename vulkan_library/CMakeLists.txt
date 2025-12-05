cmake_minimum_required(VERSION 3.28)
project(vulkan_library)

set(CMAKE_CXX_STANDARD 17)

find_package(Vulkan REQUIRED)

if(WIN32)
    include_directories(glfw/include)
else ()
    ADD_DEFINITIONS(-DVK_USE_PLATFORM_OHOS=1)
endif ()

include_directories(${Vulkan_INCLUDE_DIRS})
include_directories(include/)

file(GLOB_RECURSE SOURCE_FILES "src/*.cpp")

add_executable(vulkan_library main.cpp ${SOURCE_FILES})

target_link_libraries(vulkan_library ${Vulkan_LIBRARIES})

if(WIN32)
    target_link_libraries(vulkan_library ${PROJECT_SOURCE_DIR}/glfw/lib-mingw-w64/libglfw3.a)
endif ()
