#include <glad/glad.h>     
#include <GLFW/glfw3.h>    

#include "window.h"
#include <iostream>

static GLFWwindow* window = nullptr;
static int windowWidth = 1920;
static int windowHeight = 1080;

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void Window::Init()
{
    // ===GLFW Initialization===
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // OS X compatibility
#endif

    //===Get monitor info for windowed fullscreen===
    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);

    //===Set window hints for windowed fullscreen===
    glfwWindowHint(GLFW_RED_BITS, mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);

    //===GLFW Window Creation (windowed fullscreen)===
    window = glfwCreateWindow(mode->width, mode->height, "Modular Engine", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        exit(-1);
    }

    windowWidth = mode->width;
    windowHeight = mode->height;

    glfwMakeContextCurrent(window);

    //===GLAD Initialization===
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        exit(-1);
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set initial viewport
    glViewport(0, 0, windowWidth, windowHeight);

    // Don't capture cursor by default - let ImGui handle it
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void Window::Shutdown() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

void Window::PollEvents() {
    glfwPollEvents();
}

void Window::SwapBuffers() {
    glfwSwapBuffers(window);
}


bool Window::ShouldClose() {
    return glfwWindowShouldClose(window);
}

GLFWwindow* Window::GetGLFWWindow() {
    return window;
}

void Window::GetWindowSize(int& width, int& height) {
    width = windowWidth;
    height = windowHeight;
}