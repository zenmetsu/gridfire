#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtx/quaternion.hpp>
#include "pipeline.hpp" // For Camera definition

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
    bool toggleHUD(); // Added for HUD toggle
    bool shouldExit();
    void processImGuiInput();
    float getFrameTime() const;
    Camera getCamera() const;

private:
    GLFWwindow* window;
    Player player;
    glm::quat orientation;
    bool firstMouse;
    double lastX, lastY;
    bool lastF3State;
    bool showImGuiWindow;
    bool lastF4State; // Added for HUD toggle
    bool showHUD;     // Added for HUD toggle
    bool lastF9State;
    float frameTime;
};
