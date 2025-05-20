#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <sstream>

int main() {
    glfwInit();

    // Set up fullscreen window
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

    // Initialize ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();

    // Initialize ImGui GLFW backend
    if (!ImGui_ImplGlfw_InitForVulkan(window, true)) {
        throw std::runtime_error("Failed to initialize ImGui GLFW backend");
    }

    try {
        Device device(window);
        Swapchain swapchain(device);
        Pipeline pipeline(device, swapchain.renderPass, swapchain.extent, swapchain.MAX_FRAMES_IN_FLIGHT);
        swapchain.createFramebuffers(pipeline);
        Input input(window);

        double lastTime = glfwGetTime();
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
            double currentTime = glfwGetTime();
            float deltaTime = static_cast<float>(currentTime - lastTime);
            lastTime = currentTime;

            // Start ImGui frame
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            // Check for exit
            bool shouldExit = input.shouldExit();

            // Create ImGui debug window if enabled
            bool showImGuiWindow = input.toggleImGuiWindow();
            if (showImGuiWindow) {
                ImGui::SetNextWindowPos(ImVec2(swapchain.extent.width - 210.0f, 10.0f), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(200.0f, 150.0f), ImGuiCond_Always);
                ImGui::Begin("Debug Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoInputs);

                // Frame time and FPS
                float frameTimeMs = input.getFrameTime() * 1000.0f;
                float fps = frameTimeMs > 0.0f ? 1000.0f / frameTimeMs : 0.0f;
                ImGui::Text("Frame Time: %.2f ms", frameTimeMs);
                ImGui::Text("FPS: %.1f", fps);

                // Resolution
                ImGui::Text("Resolution: %ux%u", swapchain.extent.width, swapchain.extent.height);

                // Swapchain info
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

            // Render ImGui
            ImGui::Render();

            // Update game state
            input.updateCamera(deltaTime);
            input.processImGuiInput();
            pipeline.updateUBO(input.getCamera());
            swapchain.drawFrame(pipeline, showImGuiWindow);

            // Handle exit after rendering to ensure ImGui frame is complete
            if (shouldExit) {
                glfwSetWindowShouldClose(window, GLFW_TRUE);
            }
        }

        device.waitIdle();

        // Cleanup ImGui
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
