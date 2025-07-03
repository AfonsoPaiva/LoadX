#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
struct GLFWwindow;

namespace UI {
    void Init(GLFWwindow* window);
    void BeginFrame();
    void RenderUI();
    void EndFrame();
    void Shutdown();
}
