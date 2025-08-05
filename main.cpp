#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> 
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include "render.h"
#include "window.h"
#include "ui.h"
#include "model.h"
#include "Camera.h"
#include "Transform.h"
#include "Grid.h"
#include "Screenshot.h"

#ifdef _WIN32
#pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup")
#endif

// Global objects
Model* currentModel = nullptr;
unsigned int shaderProgram;
unsigned int gridShaderProgram;
Camera camera(glm::vec3(0.0f, 2.0f, 5.0f));
Transform modelTransform;
Grid* grid;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Window dimensions - will be updated dynamically
int SCR_WIDTH = 1920;
int SCR_HEIGHT = 1080;

// Custom cout buffer to capture debug messages
class DebugBuffer : public std::streambuf {
private:
    std::streambuf* original_cout;
    std::ostringstream buffer;

public:
    DebugBuffer() : original_cout(std::cout.rdbuf()) {
        std::cout.rdbuf(this);
    }

    ~DebugBuffer() {
        std::cout.rdbuf(original_cout);
    }

protected:
    virtual int overflow(int c) override {
        if (c != EOF) {
            buffer << static_cast<char>(c);
            original_cout->sputc(c);

            if (c == '\n') {
                std::string message = buffer.str();
                if (!message.empty() && message != "\n") {
                    // Remove the trailing newline
                    if (message.back() == '\n') {
                        message.pop_back();
                    }
                    UI::AddDebugMessage(message);
                }
                buffer.str("");
                buffer.clear();
            }
        }
        return c;
    }
};

static DebugBuffer debugBuffer;

void handleModelOperations();
void loadNewModel();
std::string detectMtlFile();
void reloadModelWithMtl();
void loadTexturesFromFolder();
void flipModelUVCoordinates();
void takeScreenshotNow();
void renderGrid();
void renderScene();
void cleanup();
std::string loadShaderFromFile(const std::string& path);
unsigned int compileShader(const std::string& source, unsigned int type);
unsigned int createShaderProgram(const std::string& vertexPath, const std::string& fragmentPath);

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (UI::cameraMovementEnabled) {
        camera.HandleMouseInput(window, xpos, ypos);
    }
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    if (UI::cameraMovementEnabled) {
        camera.ProcessMouseScroll(yoffset);
    }
}

void window_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
    UI::AddDebugMessage("Window resized to " + std::to_string(width) + "x" + std::to_string(height));
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

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

    if (vs == 0 || fs == 0) {
        glDeleteProgram(program);
        return 0;
    }

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed:\n" << infoLog << std::endl;
        glDeleteProgram(program);
        program = 0;
    }

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

// Model operation handlers
void handleModelOperations() {
    if (UI::modelSelected) {
        loadNewModel();
        UI::modelSelected = false;
    }

    if (UI::reloadModelWithMtl && currentModel) {
        reloadModelWithMtl();
        UI::reloadModelWithMtl = false;
    }

    if (UI::textureFolderSelected && currentModel) {
        loadTexturesFromFolder();
        UI::textureFolderSelected = false;
    }

    if (UI::flipUVCoordinates && currentModel) {
        flipModelUVCoordinates();
        UI::flipUVCoordinates = false;
    }
}

void loadNewModel() {
    if (currentModel) {
        delete currentModel;
        currentModel = nullptr;
    }

    try {
        std::cout << "Loading model: " << UI::selectedModelPath << std::endl;
        UI::UpdateModelLoadingProgress(0.0f, "Initializing...");

        // Auto-detect MTL file
        std::string mtlPath = detectMtlFile();

        UI::UpdateModelLoadingProgress(0.2f, "Loading model data...");
        currentModel = new Model(UI::selectedModelPath, mtlPath);

        // Auto-size model
        modelTransform.position = glm::vec3(0.0f);
        modelTransform.rotation = glm::vec3(0.0f);
        modelTransform.scale = glm::vec3(currentModel->GetRecommendedScale());

        UI::UpdateModelLoadingProgress(1.0f, "Complete!");
        std::cout << "Model loaded successfully. Applied scale: " << currentModel->GetRecommendedScale() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        UI::UpdateModelLoadingProgress(1.0f, "Failed!");
        UI::AddDebugMessage("Model loading failed: " + std::string(e.what()));
    }
}

std::string detectMtlFile() {
    if (UI::selectedModelPath.find(".obj") == std::string::npos) {
        return "";
    }

    std::string basePath = UI::selectedModelPath.substr(0, UI::selectedModelPath.find_last_of('.'));
    std::string autoMtlPath = basePath + ".mtl";

    std::ifstream mtlFile(autoMtlPath);
    if (mtlFile.good()) {
        UI::selectedMtlPath = autoMtlPath;
        std::cout << "Auto-detected MTL file: " << autoMtlPath << std::endl;
        mtlFile.close();
        return autoMtlPath;
    }
    mtlFile.close();
    return "";
}

void reloadModelWithMtl() {
    try {
        std::cout << "Reloading model with MTL file: " << UI::selectedMtlPath << std::endl;
        UI::UpdateModelLoadingProgress(0.0f, "Reloading with MTL...");

        delete currentModel;
        currentModel = new Model(UI::selectedModelPath, UI::selectedMtlPath);

        float recommendedScale = currentModel->GetRecommendedScale();
        if (modelTransform.scale.x == 1.0f && modelTransform.scale.y == 1.0f && modelTransform.scale.z == 1.0f) {
            modelTransform.scale = glm::vec3(recommendedScale);
        }

        UI::UpdateModelLoadingProgress(1.0f, "MTL Reload Complete!");
        std::cout << "Model reloaded with MTL file successfully" << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to reload model with MTL: " << e.what() << std::endl;
        UI::UpdateModelLoadingProgress(1.0f, "MTL Reload Failed!");
        UI::AddDebugMessage("MTL reload failed: " + std::string(e.what()));
    }
}

void loadTexturesFromFolder() {
    std::cout << "Loading textures from folder: " << UI::selectedTextureFolder << std::endl;
    currentModel->LoadTexturesFromFolder(UI::selectedTextureFolder);
    UI::textureUpdated = true;
    UI::AddDebugMessage("Textures loaded from folder: " + UI::selectedTextureFolder);
}

void flipModelUVCoordinates() {
    std::cout << "Flipping UV coordinates..." << std::endl;
    currentModel->FlipUVCoordinates();
    std::cout << "UV coordinates flipped. Current state: "
        << (currentModel->IsUVFlipped() ? "Flipped" : "Normal") << std::endl;
    UI::AddDebugMessage("UV coordinates flipped to: " + std::string(currentModel->IsUVFlipped() ? "Flipped" : "Normal"));
}

void takeScreenshotNow() {
    Render::ClearScreen();
    renderGrid();
    renderScene();

    std::string filename = Screenshot::GenerateScreenshotFilename();
    if (Screenshot::SaveScreenshot(filename, SCR_WIDTH, SCR_HEIGHT)) {
        std::cout << "Screenshot saved: " << filename << std::endl;
        UI::AddDebugMessage("Screenshot saved: " + filename);
    }
    else {
        UI::AddDebugMessage("Failed to save screenshot");
    }
}

void renderGrid() {
    glUseProgram(gridShaderProgram);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
        (float)SCR_WIDTH / (float)SCR_HEIGHT,
        0.1f, 100.0f);
    grid->Draw(gridShaderProgram, view, projection);
}

void renderScene() {
    if (!currentModel) return;

    glUseProgram(shaderProgram);

    glm::mat4 model;
    if (currentModel->GetModelSize() != glm::vec3(0.0f)) {
        model = modelTransform.GetModelMatrix(currentModel->GetModelCenter());
    }
    else {
        model = modelTransform.GetModelMatrix();
    }

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

void cleanup() {
    std::cout << "Shutting down engine..." << std::endl;

    if (currentModel) {
        delete currentModel;
        currentModel = nullptr;
    }

    if (grid) {
        delete grid;
        grid = nullptr;
    }

    if (shaderProgram != 0) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }

    if (gridShaderProgram != 0) {
        glDeleteProgram(gridShaderProgram);
        gridShaderProgram = 0;
    }

    UI::Shutdown();
    Window::Shutdown();

    std::cout << "Engine shutdown complete" << std::endl;
}

int main() {
    std::cout << "Initializing OpenGL Modular Engine..." << std::endl;

    Window::Init();

    GLFWwindow* window = Window::GetGLFWWindow();
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);

    Window::GetWindowSize(SCR_WIDTH, SCR_HEIGHT);

    UI::Init(window);

    glEnable(GL_DEPTH_TEST);

    std::cout << "Creating shader programs..." << std::endl;

    shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    gridShaderProgram = createShaderProgram("shaders/grid_vertex.glsl", "shaders/grid_fragment.glsl");

    if (shaderProgram == 0 || gridShaderProgram == 0) {
        std::cerr << "Failed to create shader programs!" << std::endl;
        cleanup();
        return -1;
    }

    grid = new Grid();

    std::cout << "Engine initialization complete. Ready for use." << std::endl;

    while (!Window::ShouldClose()) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        UI::UpdateStats(deltaTime);

        processInput(window);
        Window::PollEvents();

        if (UI::resetCameraPosition) {
            camera.ResetToDefault();
            UI::resetCameraPosition = false;
        }

        handleModelOperations();

        if (UI::takeScreenshot) {
            takeScreenshotNow();
            UI::takeScreenshot = false;
        }

        Render::ClearScreen();
        renderGrid();
        renderScene();

        UI::BeginFrame();
        UI::RenderUI(modelTransform, currentModel);
        UI::EndFrame();

        Window::SwapBuffers();
    }

    cleanup();
    return 0;
}