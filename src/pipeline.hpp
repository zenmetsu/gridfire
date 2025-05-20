#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <vector>

struct Camera {
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Pipeline {
    const Device& device; // Store reference to Device
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    uint32_t currentFrame; // Track current frame for UBO updates

    Pipeline(const Device& device, VkRenderPass renderPass, VkExtent2D extent, uint32_t maxFramesInFlight);
    ~Pipeline();

    void updateUBO(const Camera& camera);
};
