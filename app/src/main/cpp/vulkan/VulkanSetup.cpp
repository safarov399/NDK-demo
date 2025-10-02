#include "VulkanSetup.h"
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_android.h>
#include <android/log.h>
#include <set>

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


void createLogicalDevice() {
    // Describe the queues we need
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {graphicsQueueFamily, presentQueueFamily};

    float queuePriority = 1.0f; // Highest priority
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1; // one queue is enough for now
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // Enable device features if needed
    VkPhysicalDeviceFeatures deviceFeatures{};
    // Example: deviceFeatures.samplerAnisotropy = VK_TRUE;

    // Create device
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    // Enable extensions (e.g. swapchain)
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    // Validation layers (ignored in modern Vulkan, but must be set for compatibility)
    createInfo.enabledLayerCount = 0;

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
        LOGE("Failed to create logical device!");
        throw std::runtime_error("Failed to create logical device!");
    }

    // Retrieve queue handles
    vkGetDeviceQueue(device, graphicsQueueFamily, 0, &graphicsQueue);
    vkGetDeviceQueue(device, presentQueueFamily, 0, &presentQueue);

    LOG("Logical device + queues created successfully.");
}


void createSwapchain() {

    // 1. Query swapchain support
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, formats.data());

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data());

    // 2. Choose surface format (prefer VK_FORMAT_R8G8B8A8_UNORM)
    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& f : formats) {
        if (f.format == VK_FORMAT_R8G8B8A8_UNORM && f.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = f;
            break;
        }
    }
    swapchainImageFormat = surfaceFormat.format;

    // 3. Choose present mode (prefer MAILBOX for low latency, else FIFO)
    VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (const auto& pm : presentModes) {
        if (pm == VK_PRESENT_MODE_MAILBOX_KHR) {
            presentMode = pm;
            break;
        }
    }

    // 4. Choose swap extent (resolution)
    VkExtent2D extent = capabilities.currentExtent;
    if (extent.width == std::numeric_limits<uint32_t>::max()) {
        extent.width = 640;  // fallback
        extent.height = 480;
    }
    swapchainExtent = extent;

    // 5. Number of images
    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    // 6. Create swapchain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = swapchainImageFormat;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndices[] = {graphicsQueueFamily, presentQueueFamily};
    if (graphicsQueueFamily != presentQueueFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain) != VK_SUCCESS) {
        LOGE("Failed to create swapchain!");
        throw std::runtime_error("Failed to create swapchain!");
    }

    // 7. Retrieve swapchain images
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());

    LOG("Swapchain created with %d images.", imageCount);
}


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

std::vector<VkPresentModeKHR> querySwapChainPresentModes(VkPhysicalDevice device, VkSurfaceKHR surface) {
    std::vector<VkPresentModeKHR> presentModes;
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface,&presentModeCount , nullptr);
    if (presentModeCount != 0) {
        presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, presentModes.data());
    }
    return presentModes;
}

bool isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface) {
    VkSurfaceCapabilitiesKHR capabilities = querySwapChainCapabilities(device, surface);
    std::vector<VkSurfaceFormatKHR> formats = querySwapChainFormats(device, surface);
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
        if (!isDeviceSuitable(dev, surface)) continue;
        int score = rateDevice(dev);

        if (score > bestScore) {
            bestScore = score;
            physicalDevice = dev;

            // Find graphics & present queues
            findQueueFamilies(dev, surface, graphicsQueueFamily, presentQueueFamily);
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