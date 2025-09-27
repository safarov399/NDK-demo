//
// Created by sigma on 27.09.2025.
//

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_android.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#define LOG(...) __android_log_print(ANDROID_LOG_INFO, "NativeVulkan", __VA_ARGS__)

VkInstance instance;
VkSurfaceKHR surface;

void fill_window_accelerated(ANativeWindow* window) {
    // 1. Instance
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Native Vulkan";
    appInfo.apiVersion = VK_API_VERSION_1_0;

    const char* extensions[] = {"VK_KHR_surface", "VK_KHR_android_surface"};
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;

    vkCreateInstance(&createInfo, nullptr, &instance);

    // 2. Android surface
    VkAndroidSurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.window = window;

    vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);

    LOG("Vulkan instance + surface created!");
}

void ANativeActivity_onCreate(ANativeActivity* activity, void* savedState, size_t savedStateSize) {
    // Hook the window creation callback
    activity->callbacks->onNativeWindowCreated = [](ANativeActivity* act, ANativeWindow* window) {
        fill_window_accelerated(window);
    };
}