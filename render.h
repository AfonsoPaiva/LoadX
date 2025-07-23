#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "model.h"
#include "lighting.h"

// Global lighting variables
extern DirectionalLight dirLight;
extern PointLight pointLight;
extern SpotLight spotLight;
extern Material material;

namespace Render {
    void Init();
    void ClearScreen();
    void DrawModel(Model& model);
    void UpdateShaderLighting(unsigned int shaderProgram);
}