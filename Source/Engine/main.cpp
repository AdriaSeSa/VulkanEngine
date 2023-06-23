#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>

const uint32_t WINDOW_WIDTH = 800;
const uint32_t WINDOW_HEIGHT = 600;

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
        vkDestroyInstance(vulkanInstance, nullptr);

        glfwDestroyWindow(window);

        glfwTerminate();
    }

    // Vulkan Methods

    void InitInstance()
    {
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

        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;

        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        createInfo.enabledExtensionCount = glfwExtensionCount;
        createInfo.ppEnabledExtensionNames = glfwExtensions;
        createInfo.enabledLayerCount = 0;

        if (vkCreateInstance(&createInfo, nullptr, &vulkanInstance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

private:
    GLFWwindow* window = nullptr;
    VkInstance vulkanInstance = nullptr;
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