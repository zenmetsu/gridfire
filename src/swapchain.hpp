#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <vector>
#include <imgui.h>

struct Pipeline; // Forward declaration

struct Swapchain {
    const Device& device;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
    VkFormat imageFormat;
    VkExtent2D extent;
    std::vector<VkFramebuffer> framebuffers;
    VkRenderPass renderPass;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame;
    const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    Swapchain(const Device& device);
    ~Swapchain();

    void createFramebuffers(const Pipeline& pipeline);
    void drawFrame(const Pipeline& pipeline, bool showImGuiWindow);
    void renderImGui(VkCommandBuffer commandBuffer);
};
