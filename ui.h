#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include "Transform.h"

struct GLFWwindow;

namespace UI {
    void Init(GLFWwindow* window);
    void BeginFrame();
    void RenderUI(Transform& modelTransform);
    void EndFrame();
    void Shutdown();

    // Expose variables for external access
    extern std::string selectedModelPath;
    extern bool modelSelected;
    extern bool resetCameraPosition;
}