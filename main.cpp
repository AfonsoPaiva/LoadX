#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <iostream>
#include <fstream>
#include <sstream>
#include "render.h"
#include "window.h"
#include "ui.h"
#include "model.h"
#include "Camera.h"
#include "Transform.h"
#include "Screenshot.h"

Model* currentModel = nullptr;
unsigned int shaderProgram;
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
Transform modelTransform;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Window dimensions - will be updated dynamically
int SCR_WIDTH = 1920;
int SCR_HEIGHT = 1080;

// Mouse callback
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (UI::cameraMovementEnabled) {
        camera.HandleMouseInput(window, xpos, ypos);
    }
}

// Scroll callback
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (UI::cameraMovementEnabled) {
        camera.ProcessMouseScroll(yoffset);
    }
}

// Window resize callback
void window_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

// Process input
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Only process camera movement if enabled
    if (UI::cameraMovementEnabled) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    // Screenshot hotkey (F12)
    static bool f12WasPressed = false;
    bool f12IsPressed = (glfwGetKey(window, GLFW_KEY_F12) == GLFW_PRESS);
    if (f12IsPressed && !f12WasPressed) {
        UI::takeScreenshot = true;
    }
    f12WasPressed = f12IsPressed;
}

std::string loadShaderFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open shader file: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

unsigned int compileShader(const std::string& source, unsigned int type) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "Shader compilation error ("
            << (type == GL_VERTEX_SHADER ? "vertex" : "fragment")
            << "):\n" << infoLog << std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
}

unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode = loadShaderFromFile(vertexPath);
    std::string fragmentCode = loadShaderFromFile(fragmentPath);

    if (vertexCode.empty() || fragmentCode.empty()) {
        return 0;
    }

    unsigned int program = glCreateProgram();
    unsigned int vs = compileShader(vertexCode, GL_VERTEX_SHADER);
    unsigned int fs = compileShader(fragmentCode, GL_FRAGMENT_SHADER);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void renderScene() {
    if (currentModel) {
        glUseProgram(shaderProgram);

        glm::mat4 model = modelTransform.GetModelMatrix();

        // Camera/View transformation
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, glm::value_ptr(camera.Position));

        Render::UpdateShaderLighting(shaderProgram);
        currentModel->Draw(shaderProgram);
    }
}

int main() {
    Window::Init();

    // Set callbacks
    GLFWwindow* window = Window::GetGLFWWindow();
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    // Get actual window size
    Window::GetWindowSize(SCR_WIDTH, SCR_HEIGHT);

    UI::Init(window);

    glEnable(GL_DEPTH_TEST);

    shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program!" << std::endl;
        return -1;
    }

    while (!Window::ShouldClose()) {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Input
        processInput(window);
        Window::PollEvents();

        // Handle camera reset
        if (UI::resetCameraPosition) {
            camera.ResetToDefault();
            UI::resetCameraPosition = false;
        }

        // Handle model loading
        if (UI::modelSelected) {
            if (currentModel) delete currentModel;
            try {
                currentModel = new Model(UI::selectedModelPath);

                // Reset transform and set reasonable default scale - centered properly
                modelTransform.position = glm::vec3(0.0f, 0.0f, 0.0f);
                modelTransform.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
                modelTransform.scale = glm::vec3(1.0f, 1.0f, 1.0f);

                std::cout << "Model loaded successfully: " << UI::selectedModelPath << std::endl;
            }
            catch (const std::exception& e) {
                std::cerr << "Failed to load model: " << e.what() << std::endl;
            }
            UI::modelSelected = false;
        }

        // Handle texture folder loading
        if (UI::textureFolderSelected && currentModel) {
            currentModel->LoadTexturesFromFolder(UI::selectedTextureFolder);
            UI::textureUpdated = true;
            UI::textureFolderSelected = false;
        }

        // Handle screenshot
        if (UI::takeScreenshot) {
            // Clear screen and render scene without UI
            Render::ClearScreen();
            renderScene();

            // Take screenshot before rendering UI
            std::string filename = Screenshot::GenerateScreenshotFilename();
            Screenshot::SaveScreenshot(filename, SCR_WIDTH, SCR_HEIGHT);

            UI::takeScreenshot = false;
        }

        // Normal rendering
        Render::ClearScreen();
        renderScene();

        // Render UI
        UI::BeginFrame();
        UI::RenderUI(modelTransform, currentModel);
        UI::EndFrame();

        Window::SwapBuffers();
    }

    if (currentModel) delete currentModel;
    glDeleteProgram(shaderProgram);
    UI::Shutdown();
    Window::Shutdown();

    return 0;
}