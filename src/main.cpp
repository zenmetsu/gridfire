#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "hud_pipeline.hpp" // Added for HUD struct
#include "input.hpp"        // Added for Input class
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <sstream>
#include <glm/gtc/quaternion.hpp> // Added for glm::quat

int main() {
    glfwInit();

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height, "gridfire", monitor, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    if (!ImGui_ImplGlfw_InitForVulkan(window, true)) {
        throw std::runtime_error("Failed to initialize ImGui GLFW backend");
    }

    try {
        Device device(window);
        Swapchain swapchain(device);
        Pipeline pipeline(device, swapchain.renderPass, swapchain.extent, swapchain.MAX_FRAMES_IN_FLIGHT);
        HUDPipeline hudPipeline(device, swapchain.renderPass, swapchain.extent, swapchain.MAX_FRAMES_IN_FLIGHT);
        swapchain.createFramebuffers(pipeline);
        Input input(window);

        // Initialize bounding box
        std::vector<Object> objects = {
            {
                glm::vec3(0.0f, 0.0f, 0.0f),
                glm::quat(1.0f, 0.0f, 0.0f, 0.0f),
                glm::vec3(100.0f, 100.0f, 100.0f),
                glm::vec4(0.18f, 0.18f, 0.18f, 1.0f),
                1, // Box type
                0  // Default material
            }
        };

        double lastTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            bool shouldExit = input.shouldExit();
            bool showImGuiWindow = input.toggleImGuiWindow();
            bool showHUD = input.toggleHUD();

            if (showImGuiWindow) {
                ImGui::SetNextWindowPos(ImVec2(swapchain.extent.width - 210.0f, 10.0f), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(200.0f, 150.0f), ImGuiCond_Always);
                ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);
                float frameTimeMs = input.getFrameTime() * 1000.0f;
                float fps = frameTimeMs > 0.0f ? 1000.0f / frameTimeMs : 0.0f;
                ImGui::Text("Frame Time: %.2f ms", frameTimeMs);
                ImGui::Text("FPS: %.1f", fps);
                ImGui::Text("Resolution: %ux%u", swapchain.extent.width, swapchain.extent.height);
                ImGui::Text("Image Count: %u", swapchain.getImageCount());
                std::string presentModeStr;
                switch (swapchain.getPresentMode()) {
                    case VK_PRESENT_MODE_IMMEDIATE_KHR: presentModeStr = "Immediate"; break;
                    case VK_PRESENT_MODE_MAILBOX_KHR: presentModeStr = "Mailbox"; break;
                    case VK_PRESENT_MODE_FIFO_KHR: presentModeStr = "FIFO"; break;
                    case VK_PRESENT_MODE_FIFO_RELAXED_KHR: presentModeStr = "FIFO Relaxed"; break;
                    default: presentModeStr = "Unknown"; break;
                }
                ImGui::Text("Present Mode: %s", presentModeStr.c_str());
                ImGui::End();
            }

            ImGui::Render();

            input.updateCamera(deltaTime);
            input.processImGuiInput();

            // Update HUD data
            HUD hud;
            hud.crosshairPos = glm::vec2(0.0f, 0.0f);
            hud.crosshairSize = 32.0f / swapchain.extent.height;
            hud.crosshairColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
            hud.windowPos = glm::vec2(0.0f, -0.9f);
            hud.windowSize = glm::vec2(0.1f * swapchain.extent.width / swapchain.extent.height, 0.1f);
            hud.windowAlpha = 0.5f;
            hud.playerPos = input.getCamera().position;
            hud.playerForward = input.getCamera().forward;
            hud.playerRotation = glm::quat(input.getCamera().view);

            pipeline.updateUBO(input.getCamera(), objects);
            hudPipeline.updateUBO(input.getCamera(), hud, showHUD);
            swapchain.drawFrame(pipeline, hudPipeline, showImGuiWindow, showHUD);

            if (shouldExit) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }

        device.waitIdle();

        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
