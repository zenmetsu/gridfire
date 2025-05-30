#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "pipeline.hpp" // Added for Camera definition

struct Player {
    glm::vec3 position;
    glm::vec3 forward;
    glm::vec3 up;
    glm::mat4 view;
    glm::mat4 proj;
};

class Input {
public:
    Input(GLFWwindow* window);
    void updateCamera(float deltaTime);
    bool toggleImGuiWindow();
    bool shouldExit();
    void processImGuiInput();
    float getFrameTime() const;
    Camera getCamera() const; // Returns Camera for compatibility with pipeline.hpp

private:
    GLFWwindow* window;
    Player player; // Renamed from camera
    glm::quat orientation; // Quaternion for player orientation
    bool firstMouse;
    double lastX, lastY;
    bool lastF3State;
    bool showImGuiWindow;
    bool lastF9State;
    float frameTime;
};
