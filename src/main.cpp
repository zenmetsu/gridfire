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

            // Create ImGui window if enabled
            bool showImGuiWindow = input.toggleImGuiWindow();
            if (showImGuiWindow) {
                ImGui::SetNextWindowPos(ImVec2(swapchain.extent.width - 210.0f, 10.0f), ImGuiCond_Always);
                ImGui::SetNextWindowSize(ImVec2(200.0f, 100.0f), ImGuiCond_Always);
                ImGui::Begin("Debug Window", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
                ImGui::Text("Hello World");
                ImGui::End();
            }

            // Render ImGui
            ImGui::Render();

            input.updateCamera(deltaTime);
            input.processImGuiInput();
            pipeline.updateUBO(input.getCamera());
            swapchain.drawFrame(pipeline, showImGuiWindow);
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
