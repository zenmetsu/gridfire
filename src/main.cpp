#include "device.hpp"
#include "swapchain.hpp"
#include "pipeline.hpp"
#include "input.hpp"
#include <GLFW/glfw3.h>
#include <stdexcept>
#include <iostream>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "gridfire", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window");
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

            input.updateCamera(deltaTime);
            pipeline.updateUBO(input.getCamera());
            swapchain.drawFrame(pipeline);
        }

        device.waitIdle();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
