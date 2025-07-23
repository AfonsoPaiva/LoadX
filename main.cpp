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

Model* currentModel = nullptr;
unsigned int shaderProgram;

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

int main() {
    Window::Init();
    UI::Init(Window::GetGLFWWindow());

    glEnable(GL_DEPTH_TEST);

    shaderProgram = createShaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl");
    if (shaderProgram == 0) {
        std::cerr << "Failed to create shader program!" << std::endl;
        return -1;
    }

    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 projection = glm::perspective(glm::radians(45.0f),
        (float)800 / (float)600,
        0.1f, 100.0f);

    while (!Window::ShouldClose()) {
        Window::PollEvents();
        Window::ProcessInput();

        Render::ClearScreen();

        if (UI::modelSelected) {
            if (currentModel) delete currentModel;
            try {
                currentModel = new Model(UI::selectedModelPath);
            }
            catch (const std::exception& e) {
                std::cerr << "Failed to load model: " << e.what() << std::endl;
            }
            UI::modelSelected = false;
        }

        if (currentModel) {
            glUseProgram(shaderProgram);

            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
            glUniform3f(glGetUniformLocation(shaderProgram, "viewPos"), 0.0f, 0.0f, 3.0f);

            Render::UpdateShaderLighting(shaderProgram);
            currentModel->Draw();
        }

        UI::BeginFrame();
        UI::RenderUI();
        UI::EndFrame();

        Window::SwapBuffers();
    }

    if (currentModel) delete currentModel;
    glDeleteProgram(shaderProgram);
    UI::Shutdown();
    Window::Shutdown();

    return 0;
}