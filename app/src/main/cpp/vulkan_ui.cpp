//
// Created by sigma on 27.09.2025.
//

#include <vulkan/vulkan.h>
#include "vulkan/VulkanSetup.h"
#include <vulkan/vulkan_android.h>
#include <android/native_activity.h>
#include <android/native_window.h>
#include <android/log.h>
#include <vector>
#include <string>
#include <cstring> // strcmp
#include <limits>

void drawFrame();


void fill_window_accelerated(ANativeWindow *window) {
    createInstance();
    LOG("Inside vulkan_ui.cpp. Vulkan instance created!");

    createSurface(window);
    LOG("Vulkan surface created!");


    pickPhysicalDevice();
    LOG("Inside vulkan_ui.cpp. Physical device picked.");

    //    TODO() Not finished yet
    createLogicalDevice();
    LOG("Inside vulkan_ui.cpp. Logical device created.");
//
//    createSwapchain();
//    LOG("Inside vulkan_ui.cpp. Created Swapchain.");
//
//    createImageViews();
//    LOG("Inside vulkan_ui.cpp. Created image views.");
//
//    createRenderPass();
//    LOG("Inside vulkan_ui.cpp. Created render pass.");
//
//    createGraphicsPipeline();
//    LOG("Inside vulkan_ui.cpp. Created graphics pipeline.");
//
//    createFramebuffers();
//    LOG("Inside vulkan_ui.cpp. Created frame buffer.");
//
//    createCommandPool();
//    LOG("Inside vulkan_ui.cpp. Created command pool.");
//
//    createCommandBuffers();
//    LOG("Inside vulkan_ui.cpp. Created command buffers.");
//
//    createSyncObjects();
//    LOG("Inside vulkan_ui.cpp. Created sync objects.");
}

void ANativeActivity_onCreate(ANativeActivity *activity, void *savedState, size_t savedStateSize) {
    // Hook the window creation callback
    activity->callbacks->onNativeWindowCreated = [](ANativeActivity *act, ANativeWindow *window) {
        fill_window_accelerated(window);
    };
}