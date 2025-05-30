#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <vector>

struct Pipeline;
struct HUDPipeline; // Added for HUD pipeline

struct Swapchain {
    const Device& device;
    VkSwapchainKHR swapchain;
    std::vector<VkImage> images;
    VkFormat imageFormat;
    VkExtent2D extent;
    std::vector<VkImageView> imageViews;
    VkRenderPass renderPass;
    std::vector<VkFramebuffer> framebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    uint32_t currentFrame;
    VkPresentModeKHR presentMode;

    static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

    Swapchain(const Device& device);
    ~Swapchain();

    void createFramebuffers(const Pipeline& pipeline);
    void renderImGui(VkCommandBuffer commandBuffer);
    void drawFrame(const Pipeline& pipeline, const HUDPipeline& hudPipeline, bool showImGuiWindow, bool showHUD); // Updated signature
    uint32_t getImageCount() const;
    VkPresentModeKHR getPresentMode() const;
};
