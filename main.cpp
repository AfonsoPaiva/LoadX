#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

//===Global Variables===
bool changeColor = false;

//screen_size
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;


//===ImGui Initialization===
void initImGui(GLFWwindow* window) {
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330 core");

}

int main() {


	// ===GLFW Initialization===
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // OS X compatibility
#endif

	//===GLFW Window Creation===
	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Modular Engine", NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	//===initialize ImGui===
	initImGui(window);


	//===GLAD Initialization===
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
	

	//===Render Loop===
	while(!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		//render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		if (changeColor)
			glClearColor(0.3f, 0.1f, 0.2f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		//imgui frame start
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		
		

		//imgui window
		ImGui::SetNextWindowPos(ImVec2(58, 48), ImGuiCond_Once);
		ImGui::Begin("Basic Functionalities");
		ImGui::Text("This is a basic ImGui window with some functionalities.");
		ImGui::Checkbox("Checkbox", &changeColor);
		ImGui::End();

		//imgui render
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



		//swap buffers
		glfwSwapBuffers(window);
		
		//poll events
		glfwPollEvents();
	}

	//===Cleanup ImGui===
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	
	
	//===Cleanup GLFW===
	glfwDestroyWindow(window);



	//terminate
	glfwTerminate();
	return 0;
}

//===Process all input===
void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}
}

//===Resize callback===
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	//make sure the viewport matches the new window dimensions
	glViewport(0, 0, width, height);
}
