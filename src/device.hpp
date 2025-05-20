#pragma once
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <vector>

struct Device {
    VkInstance instance;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    uint32_t graphicsFamily;
    uint32_t presentFamily;

    Device(GLFWwindow* window);
    ~Device();

    void waitIdle();
};
