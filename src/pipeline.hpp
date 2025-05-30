#pragma once
#include "device.hpp"
#include <vulkan/vulkan.h>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp> // Added for glm::quat
#include <vector>

struct Camera {
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::mat4 view;
    glm::mat4 proj;
};

struct Object {
    glm::vec3 position;
    glm::quat rotation;
    glm::vec3 scale;
    glm::vec4 color;
    int type; // 0: sphere, 1: box
    int materialIndex;
};

struct HUD; // Forward declaration for HUDPipeline

struct Pipeline {
    const Device& device;
    VkPipeline graphicsPipeline;
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSet> descriptorSets;
    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<VkBuffer> objectBuffers;
    std::vector<VkDeviceMemory> objectBuffersMemory;
    uint32_t currentFrame;

    Pipeline(const Device& device, VkRenderPass renderPass, VkExtent2D extent, uint32_t maxFramesInFlight);
    ~Pipeline();

    void updateUBO(const Camera& camera, const std::vector<Object>& objects);
};
