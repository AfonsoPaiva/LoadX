#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "lighting.h"

// Global lighting variables
extern DirectionalLight dirLight;
extern PointLight pointLight;
extern SpotLight spotLight;
extern Material material;

namespace Render {
    void ClearScreen();
    void UpdateShaderLighting(unsigned int shaderProgram);
}