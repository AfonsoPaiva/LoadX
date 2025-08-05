#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace Window {
    void Init();
    void Shutdown();
    void PollEvents();
    void SwapBuffers();
    bool ShouldClose();
    GLFWwindow* GetGLFWWindow();
    void GetWindowSize(int& width, int& height);
#ifdef _WIN32
    void SetWindowIcon();
#endif
}