#include "input.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>
#include <imgui_impl_glfw.h>

Input::Input(GLFWwindow* window) : window(window), firstMouse(true), yaw(-90.0f), pitch(0.0f), roll(0.0f), lastF3State(false), showImGuiWindow(false) {
    camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
    camera.forward = glm::vec3(0.0f, 0.0f, -1.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    camera.view = glm::lookAt(camera.position, camera.position + camera.forward, camera.up);
    camera.proj = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Input::updateCamera(float deltaTime) {
    // Keyboard input
    float moveSpeed = 5.0f;
    glm::vec3 right = glm::normalize(glm::cross(camera.forward, camera.up));

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        camera.position += camera.forward * moveSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        camera.position -= camera.forward * moveSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        camera.position -= right * moveSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        camera.position += right * moveSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        camera.position += camera.up * moveSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        camera.position -= camera.up * moveSpeed * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        roll -= 45.0f * deltaTime;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS && !ImGui::GetIO().WantCaptureKeyboard) {
        roll += 45.0f * deltaTime;
    }

    // Mouse input
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (!ImGui::GetIO().WantCaptureMouse) {
        float xoffset = static_cast<float>(xpos - lastX);
        float yoffset = static_cast<float>(lastY - ypos);
        lastX = xpos;
        lastY = ypos;

        float sensitivity = 0.1f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        pitch = std::clamp(pitch, -89.0f, 89.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        camera.forward = glm::normalize(direction);

        // Apply roll
        glm::mat4 rollMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(roll), camera.forward);
        camera.up = glm::vec3(rollMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));
    } else {
        lastX = xpos;
        lastY = ypos;
    }

    camera.view = glm::lookAt(camera.position, camera.position + camera.forward, camera.up);
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

void Input::processImGuiInput() {
    ImGui_ImplGlfw_NewFrame();
}

Camera Input::getCamera() const {
    return camera;
}
