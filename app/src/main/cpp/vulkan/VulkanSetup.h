#pragma once

#include <vulkan/vulkan.h>
#include <android/native_window.h>
#include <android/log.h>

#define LOG(...)  __android_log_print(ANDROID_LOG_INFO, "NativeVulkan", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "NativeVulkan", __VA_ARGS__)

void createInstance();

void createSurface(ANativeWindow *window);

void pickPhysicalDevice();

void createLogicalDevice();

void createSwapchain();

void createImageViews();

void createRenderPass();

void createGraphicsPipeline();

void createFramebuffers();

void createCommandPool();

void createCommandBuffers();

void createSyncObjects();