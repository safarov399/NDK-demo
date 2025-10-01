#include "VulkanSetup.h"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_android.h>
#include <android/log.h>

// define globals here
VkQueue graphicsQueue;
VkQueue presentQueue;
VkDevice device;
VkFormat swapchainImageFormat;
VkInstance instance;
VkPipeline graphicsPipeline;
VkExtent2D swapchainExtent;
VkRenderPass renderPass;
VkSurfaceKHR surface;
VkCommandPool commandPool;
VkSwapchainKHR swapchain;
VkPipelineLayout pipelineLayout;
VkPhysicalDevice physicalDevice;


uint32_t graphicsQueueFamily;
uint32_t presentQueueFamily;

const std::vector<const char *> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
};
std::vector<VkSemaphore> imageAvailableSemaphores;
std::vector<VkSemaphore> renderFinishedSemaphores;
std::vector<VkFence> inFlightFences;
std::vector<VkImage> swapchainImages;
std::vector<VkImageView> swapchainImageViews;
std::vector<VkFramebuffer> swapchainFramebuffers;
std::vector<VkCommandBuffer> commandBuffers;


void createLogicalDevice() {}

void createSwapchain() {}

void createImageViews() {}

void createRenderPass() {}

void createGraphicsPipeline() {}

void createFramebuffers() {}

void createCommandPool() {}

void createCommandBuffers() {}

void createSyncObjects() {}


void findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface, uint32_t &outGraphicsIndex, uint32_t &outPresentIndex) {
    outGraphicsIndex = std::numeric_limits<uint32_t>::max();
    outPresentIndex = std::numeric_limits<uint32_t>::max();

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        // graphics support?
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            if (outGraphicsIndex == std::numeric_limits<uint32_t>::max())
                outGraphicsIndex = i;
        }

        // present support (surface-specific)
        VkBool32 presentSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
        if (presentSupport) {
            if (outPresentIndex == std::numeric_limits<uint32_t>::max())
                outPresentIndex = i;
        }

        if (outGraphicsIndex != std::numeric_limits<uint32_t>::max()
            && outPresentIndex != std::numeric_limits<uint32_t>::max()) {
            break; // found both
        }
    }
}

bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
    uint32_t extCount = 0;
    VkResult r = vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, nullptr);
    if (r != VK_SUCCESS) {
        LOG("vkEnumerateDeviceExtensionProperties failed: %d", r);
        return false;
    }

    std::vector<VkExtensionProperties> available(extCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extCount, available.data());

    for (const char *required: deviceExtensions) {
        bool found = false;
        for (const auto &avail: available) {
            if (std::strcmp(avail.extensionName, required) == 0) {
                found = true;
                break;
            }
        }
        if (!found) {
            LOG("Missing device extension: %s", required);
            return false;
        }
    }
    return true;
}

VkSurfaceCapabilitiesKHR querySwapChainCapabilities(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities{};
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &capabilities);
    return capabilities;
}

std::vector<VkSurfaceFormatKHR> querySwapChainFormats(VkPhysicalDevice device, VkSurfaceKHR surface) {
    std::vector<VkSurfaceFormatKHR> formats;
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, formats.data());
    }

    return formats;
}

std::vector<VkPresentModeKHR> querySwapChainPresentModes(VkPhysicalDevice device, VkSurfaceKHR surface) {}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    LOG("Inside VulkanSetup.cpp, inside isDeviceSuitable, before capabilities");
    VkSurfaceCapabilitiesKHR capabilities = querySwapChainCapabilities(device, surface);

    LOG("Inside VulkanSetup.cpp, inside isDeviceSuitable, before formats");
    std::vector<VkSurfaceFormatKHR> formats = querySwapChainFormats(device, surface);

    LOG("Inside VulkanSetup.cpp, inside isDeviceSuitable, before presentModes");
    std::vector<VkPresentModeKHR> presentModes = querySwapChainPresentModes(device, surface);
    // 1) Queue families
    uint32_t graphicsIndex, presentIndex;

    findQueueFamilies(device, surface, graphicsIndex, presentIndex);
    if (graphicsIndex == std::numeric_limits<uint32_t>::max() ||
        presentIndex == std::numeric_limits<uint32_t>::max()) {
        LOG("Device lacks required queue families (graphics/present).");
        return false;
    }

    // 2) Device extensions (we need swapchain)
    if (!checkDeviceExtensionSupport(device)) {
        return false;
    }

    // 3) Swapchain support: at least one surface format and present mode
    if (formats.empty() || presentModes.empty()) {
        LOG("Device has no usable swapchain formats or present modes.");
        return false;
    }

    // 4) Optionally: check required features (for a simple clear/color we don't need much)
    VkPhysicalDeviceFeatures supportedFeatures;
    vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
    // if you needed e.g. geometryShader, sampleRates, etc. check here:
    // if (!supportedFeatures.geometryShader) return false;

    return true;
}

void createInstance() {
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Native Vulkan";
    appInfo.apiVersion = VK_API_VERSION_1_0;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);


    const char *extensions[] = {
            VK_KHR_SURFACE_EXTENSION_NAME,
            VK_KHR_ANDROID_SURFACE_EXTENSION_NAME
    };
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = 2;
    createInfo.ppEnabledExtensionNames = extensions;
    createInfo.enabledLayerCount = 0;

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);

    if (result != VK_SUCCESS) {
        LOGE("Failed to create Vulkan instance! Error code: %d", result);
        throw std::runtime_error("Failed to create Vulkan instance!");
    }

    LOG("Vulkan instance created successfully.");

}

void createSurface(ANativeWindow *window) {
    VkAndroidSurfaceCreateInfoKHR surfaceInfo{};
    surfaceInfo.sType = VK_STRUCTURE_TYPE_ANDROID_SURFACE_CREATE_INFO_KHR;
    surfaceInfo.window = window;
    VkResult result = vkCreateAndroidSurfaceKHR(instance, &surfaceInfo, nullptr, &surface);

    if (result != VK_SUCCESS) {
        LOG("Failed to create Android surface, error code: %d", result);
        throw std::runtime_error("Failed to create Vulkan surface!");
    }

    LOG("Surface created successfully.");
}

int rateDevice(VkPhysicalDevice device) {
    VkPhysicalDeviceProperties props;
    vkGetPhysicalDeviceProperties(device, &props);

    int score = 0;
    // Prefer discrete GPUs
    if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }
    // Give a little preference to higher max image dimension (heuristic)
    score += static_cast<int>(props.limits.maxImageDimension2D / 1024);

    return score;
}

void pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        LOG("No Vulkan-capable GPUs found.");
        return;
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    int bestScore = -1;
    for (auto dev: devices) {
        LOG("Inside VulkanSetup.cpp, inside, pickPhysicalDevice, inside for, before everything");

        if (!isDeviceSuitable(dev, surface)) continue;

        LOG("Inside VulkanSetup.cpp, inside for, before rateDevice");
        int score = rateDevice(dev);

        if (score > bestScore) {
            bestScore = score;
            physicalDevice = dev;

            // Find graphics & present queues
            LOG("Inside VulkanSetup.cpp, inside for, inside if, before findQueueFamilies");

            findQueueFamilies(dev, surface, graphicsQueueFamily, presentQueueFamily);
            LOG("Inside VulkanSetup.cpp, inside for, inside if, after findQueueFamilies");

        }

    }

    if (physicalDevice == VK_NULL_HANDLE) {
        LOG("No suitable GPU found!");
    } else {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevice, &props);
        LOG("Chosen GPU: %s", props.deviceName);
    }
}











