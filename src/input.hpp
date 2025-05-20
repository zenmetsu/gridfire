#pragma once
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "pipeline.hpp"

struct Input {
    Input(GLFWwindow* window);
    void updateCamera(float deltaTime);
    Camera getCamera() const;
    bool toggleImGuiWindow();
    void processImGuiInput();
    bool shouldExit();
    float getFrameTime() const;

private:
    GLFWwindow* window;
    Camera camera;
    double lastX, lastY;
    bool firstMouse;
    float yaw, pitch, roll;
    bool lastF3State;
    bool showImGuiWindow;
    bool lastF9State;
    float frameTime;
};
