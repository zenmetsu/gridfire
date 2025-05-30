#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> // For glm::quat
#include <vector>

struct Camera; // Forward declaration

struct HUD {
    glm::vec2 crosshairPos;   // NDC coordinates
    float crosshairSize;
    glm::vec4 crosshairColor;
    glm::vec2 windowPos;      // Bottom-center in NDC
    glm::vec2 windowSize;     // NDC dimensions
    float windowAlpha;
    glm::vec3 playerPos;
    glm::vec3 playerForward;
    glm::quat playerRotation; // Quaternion
};

struct HUDPipeline {
    const Device& device;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<VkBuffer> hudBuffers;
    std::vector<VkDeviceMemory> hudBuffersMemory;
    uint32_t currentFrame;

    HUDPipeline(const Device& device, VkRenderPass renderPass, VkExtent2D extent, uint32_t maxFramesInFlight);
    ~HUDPipeline();

    void updateUBO(const Camera& camera, const HUD& hud, bool showHUD);
};
