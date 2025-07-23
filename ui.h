#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>

struct GLFWwindow;

namespace UI {
    void Init(GLFWwindow* window);
    void BeginFrame();
    void RenderUI();
    void EndFrame();
    void Shutdown();

    // Add these to expose file path
    extern std::string selectedModelPath;
    extern bool modelSelected;
}