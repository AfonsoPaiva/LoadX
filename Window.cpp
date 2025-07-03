#include <glad/glad.h>     
#include <GLFW/glfw3.h>    

#include "window.h"
#include <iostream>

const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

static GLFWwindow* window = nullptr;

void framebuffer_size_callback(GLFWwindow*, int width, int height) {
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

	//===GLFW Window Creation===
	window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Modular Engine", NULL, NULL);
	if (!window) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);


	//===GLAD Initialization===
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		exit(-1);
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
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

void Window::ProcessInput() {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

bool Window::ShouldClose() {
	return glfwWindowShouldClose(window);
}

GLFWwindow* Window::GetGLFWWindow() {
	return window;
}