#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "ValidationLayers.h"

#include <Windows.h> // To show an error in a box even in release.
#include <optional> // To use std::optional. Black magic.   


const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

struct QueueFamilyIndices 
{
    std::optional<uint32_t> graphicsFamily;
    bool IsComplete() 
    {
        return graphicsFamily.has_value();
    }
};

class HelloTriangleApplication {
public:
    void Run() 
    {
        InitWindow();
        InitVulkan();
        MainLoop();
        CleanUp();
    }

private:
    void InitWindow()
    {
        glfwInit();

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Vulkan Engine", nullptr, nullptr);
    }

    void InitVulkan() 
    {
        InitInstance();
        SetUpDebugMessenger();
        PickPhysicalDevice();
    }

    void MainLoop() 
    {
        while (!glfwWindowShouldClose(window)) 
        {
            glfwPollEvents();
        }
    }

    void CleanUp() 
    {
        if (enableValidationLayers) {
            DestroyDebugUtilsMessengerEXT(vulkanInstance, debugMessenger, nullptr);
        }

        vkDestroyInstance(vulkanInstance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    // Vulkan Methods

    void InitInstance()
    {
        if (enableValidationLayers && !CheckValidationLayersSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_API_VERSION(0,1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_API_VERSION(0,1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0;

        VkInstanceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = GetRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers) 
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            PopulateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        }
        else 
        {
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    void PickPhysicalDevice()
    {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, nullptr);
        if (deviceCount != 0) 
        {
            MessageBox(NULL, TEXT("Vulkan is not supported by your graphics card!"), TEXT("Vulkan not supported."), MB_ICONERROR | MB_OK);
            throw std::runtime_error("failed to find GPUs with Vulkan support!");
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(vulkanInstance, &deviceCount, devices.data());

        for (const auto& device : devices) 
        {
            if (IsDeviceSuitable(device)) 
            {
                physicalDevice = device;
                break;
            }
        }

        if (physicalDevice == VK_NULL_HANDLE) 
        {
            throw std::runtime_error("failed to find a suitable GPU!");
        }
    }

    bool IsDeviceSuitable(VkPhysicalDevice device) 
    {
        QueueFamilyIndices indices = FindQueueFamilies(device);

        return indices.IsComplete();
        // TODO: Check Vulkan tutorial to pick the "best" device based on scroe + necessary features (geometry shaders)
    }

    bool CheckValidationLayersSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr); // Get layer count

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data()); // Fill data

        for (const char* layerName : validationLayers) 
        {
            bool layerFound = false;

            for (const auto& layerProperties : availableLayers) 
            {
                if (strcmp(layerName, layerProperties.layerName) == 0) // Check if layer is in availableLayers
                {
                    layerFound = true;
                    break;
                }
            }

            if (!layerFound) {
                return false;
            }
        }

        return true;
    }

    std::vector<const char*> GetRequiredExtensions() 
    {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers) 
        {
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData) 
    {

        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void SetUpDebugMessenger()
    {
        if (!enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(vulkanInstance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
    {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr)
        {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        }
        else 
        {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) 
        {
            func(instance, debugMessenger, pAllocator);
        }
    }

    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;
        // Logic to find queue family indices to populate struct with

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) 
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) 
            {
                indices.graphicsFamily = i;
            }

            if (indices.IsComplete()) {
                break;
            }
            i++;
        }

        return indices;
    }

private:
    GLFWwindow* window = nullptr;
    VkInstance vulkanInstance = nullptr;
    VkDebugUtilsMessengerEXT debugMessenger = nullptr;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
};

int main()
{
    HelloTriangleApplication app;

    try 
    {
        app.Run();
    }
    catch (const std::exception& e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}