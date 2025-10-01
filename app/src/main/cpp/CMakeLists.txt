cmake_minimum_required(VERSION 3.22.1)
project(NativeActivity LANGUAGES C CXX)

# This removes symbol info and debugging stuff
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--strip-all")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -s -fvisibility=hidden")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s -fvisibility=hidden -fvisibility-inlines-hidden")

add_library(native-activity SHARED main.cpp vulkan_ui.cpp vulkan/VulkanSetup.cpp)
# Vulkan loader is part of the NDK
find_library(vulkan-lib vulkan)
target_link_libraries(native-activity
        android
        log
        ${vulkan-lib}
)