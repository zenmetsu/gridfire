#define GLM_ENABLE_EXPERIMENTAL
#include "input.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <algorithm>
#include <imgui_impl_glfw.h>

Input::Input(GLFWwindow* window) 
    : window(window), 
      firstMouse(true), 
      lastF3State(false), 
      showImGuiWindow(false), 
      lastF4State(false), // Initialize
      showHUD(true),      // HUD on by default
      lastF9State(false), 
      frameTime(0.0f) {
    player.position = glm::vec3(0.0f, 0.0f, 0.0f);
    player.forward = glm::vec3(0.0f, 0.0f, -1.0f);
    player.up = glm::vec3(0.0f, 1.0f, 0.0f);
    orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    glm::mat4 rotation = glm::toMat4(orientation);
    player.view = glm::inverse(glm::translate(glm::mat4(1.0f), player.position) * rotation);
    player.proj = glm::perspective(glm::radians(80.0f), 800.0f / 600.0f, 0.1f, 100.0f);
    player.proj[1][1] *= -1.0f;

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::updateCamera(float deltaTime) {
    frameTime = deltaTime;

    // Derive local axes from quaternion
    glm::mat4 rotation = glm::toMat4(orientation);
    glm::vec3 right = glm::normalize(glm::vec3(rotation[0]));    // Local X
    player.up = glm::normalize(glm::vec3(rotation[1]));          // Local Y
    player.forward = glm::normalize(glm::vec3(rotation[2]));     // Local Z

    // Keyboard input for movement
    float moveSpeed = 5.0f;
    // Corrected translations
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        player.position -= player.forward * moveSpeed * deltaTime; // Forward
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        player.position += player.forward * moveSpeed * deltaTime; // Backward
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        player.position += right * moveSpeed * deltaTime; // Left (restored)
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        player.position -= right * moveSpeed * deltaTime; // Right (restored)
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        player.position += player.up * moveSpeed * deltaTime; // Up
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        player.position -= player.up * moveSpeed * deltaTime; // Down
    }

    // Roll input (Q/E)
    float rollSpeed = 4.5f; // Reduced to 5% of 90.0f
    float rollAngle = 0.0f;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
        rollAngle += rollSpeed * deltaTime; // Counterclockwise (will be negated)
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        rollAngle -= rollSpeed * deltaTime; // Clockwise (will be negated)
    }

    // Mouse input for rotation
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (!ImGui::GetIO().WantCaptureMouse) {
        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(lastY - ypos); // Reversed Y
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        // Convert to radians
        float yawAngle = glm::radians(xoffset);   // Mouse X for yaw
        float pitchAngle = glm::radians(yoffset); // Mouse Y for pitch

        // Create quaternions for rotations in local space
        glm::quat yawQuat = glm::angleAxis(yawAngle, player.up);        // Yaw (correct)
        glm::quat pitchQuat = glm::angleAxis(-pitchAngle, right);       // Pitch (reversed)
        glm::quat rollQuat = glm::angleAxis(-rollAngle, player.forward); // Roll (reversed)

        // Apply rotations: yaw, pitch, roll
        orientation = glm::normalize(yawQuat * pitchQuat * rollQuat * orientation);

        // Recompute axes to ensure orthogonality
        rotation = glm::toMat4(orientation);
        right = glm::normalize(glm::vec3(rotation[0]));
        player.up = glm::normalize(glm::vec3(rotation[1]));
        player.forward = glm::normalize(glm::vec3(rotation[2]));
    } else {
        lastX = xpos;
        lastY = ypos;
    }

    // Update view matrix: inverse of (translate * rotate)
    player.view = glm::inverse(glm::translate(glm::mat4(1.0f), player.position) * glm::toMat4(orientation));
}

bool Input::toggleHUD() {
    bool currentF4State = glfwGetKey(window, GLFW_KEY_F4) == GLFW_PRESS;
    bool toggle = !lastF4State && currentF4State;
    lastF4State = currentF4State;
    if (toggle) {
        showHUD = !showHUD;
    }
    return showHUD;
}

bool Input::toggleImGuiWindow() {
    bool currentF3State = glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS;
    bool toggle = !lastF3State && currentF3State;
    lastF3State = currentF3State;
    if (toggle) {
        showImGuiWindow = !showImGuiWindow;
    }
    return showImGuiWindow;
}

bool Input::shouldExit() {
    bool currentF9State = glfwGetKey(window, GLFW_KEY_F9) == GLFW_PRESS;
    bool toggle = !lastF9State && currentF9State;
    lastF9State = currentF9State;
    return toggle;
}

void Input::processImGuiInput() {
    ImGui_ImplGlfw_NewFrame();
}

float Input::getFrameTime() const {
    return frameTime;
}

Camera Input::getCamera() const {
    Camera camera;
    camera.position = player.position;
    camera.forward = player.forward;
    camera.up = player.up;
    camera.view = player.view;
    camera.proj = player.proj;
    return camera;
}
