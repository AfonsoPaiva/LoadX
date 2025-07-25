#include "render.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Initialize lighting with default values
DirectionalLight dirLight = {
    .enabled = true,
    .direction = glm::vec3(-0.2f, -1.0f, -0.3f),
    .ambient = glm::vec3(0.2f, 0.2f, 0.2f),
    .diffuse = glm::vec3(0.5f, 0.5f, 0.5f),
    .specular = glm::vec3(1.0f, 1.0f, 1.0f)
};

PointLight pointLight = {
    .enabled = true,
    .position = glm::vec3(1.2f, 1.0f, 2.0f),
    .ambient = glm::vec3(0.05f, 0.05f, 0.05f),
    .diffuse = glm::vec3(0.8f, 0.8f, 0.8f),
    .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    .constant = 1.0f,
    .linear = 0.09f,
    .quadratic = 0.032f
};

SpotLight spotLight = {
    .enabled = true,
    .position = glm::vec3(0.0f, 0.0f, 3.0f),
    .direction = glm::vec3(0.0f, 0.0f, -1.0f),
    .ambient = glm::vec3(0.0f, 0.0f, 0.0f),
    .diffuse = glm::vec3(1.0f, 1.0f, 1.0f),
    .specular = glm::vec3(1.0f, 1.0f, 1.0f),
    .cutOff = glm::cos(glm::radians(12.5f)),
    .outerCutOff = glm::cos(glm::radians(15.0f)),
    .constant = 1.0f,
    .linear = 0.09f,
    .quadratic = 0.032f
};

Material material = {
    .ambient = glm::vec3(1.0f, 0.5f, 0.31f),
    .diffuse = glm::vec3(1.0f, 0.5f, 0.31f),
    .specular = glm::vec3(0.5f, 0.5f, 0.5f),
    .shininess = 32.0f
};

void Render::ClearScreen() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Render::UpdateShaderLighting(unsigned int shaderProgram) {
    glUseProgram(shaderProgram);

    // Light enable/disable uniforms
    glUniform1i(glGetUniformLocation(shaderProgram, "dirLightEnabled"), dirLight.enabled);
    glUniform1i(glGetUniformLocation(shaderProgram, "pointLightEnabled"), pointLight.enabled);
    glUniform1i(glGetUniformLocation(shaderProgram, "spotLightEnabled"), spotLight.enabled);

    // Directional light
    glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.direction"), dirLight.direction.x, dirLight.direction.y, dirLight.direction.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.ambient"), dirLight.ambient.x, dirLight.ambient.y, dirLight.ambient.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.diffuse"), dirLight.diffuse.x, dirLight.diffuse.y, dirLight.diffuse.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "dirLight.specular"), dirLight.specular.x, dirLight.specular.y, dirLight.specular.z);

    // Point light
    glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.position"), pointLight.position.x, pointLight.position.y, pointLight.position.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.ambient"), pointLight.ambient.x, pointLight.ambient.y, pointLight.ambient.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.diffuse"), pointLight.diffuse.x, pointLight.diffuse.y, pointLight.diffuse.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "pointLight.specular"), pointLight.specular.x, pointLight.specular.y, pointLight.specular.z);
    glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.constant"), pointLight.constant);
    glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.linear"), pointLight.linear);
    glUniform1f(glGetUniformLocation(shaderProgram, "pointLight.quadratic"), pointLight.quadratic);

    // Spot light
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.position"), spotLight.position.x, spotLight.position.y, spotLight.position.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.direction"), spotLight.direction.x, spotLight.direction.y, spotLight.direction.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.ambient"), spotLight.ambient.x, spotLight.ambient.y, spotLight.ambient.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.diffuse"), spotLight.diffuse.x, spotLight.diffuse.y, spotLight.diffuse.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "spotLight.specular"), spotLight.specular.x, spotLight.specular.y, spotLight.specular.z);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.cutOff"), spotLight.cutOff);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.outerCutOff"), spotLight.outerCutOff);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.constant"), spotLight.constant);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.linear"), spotLight.linear);
    glUniform1f(glGetUniformLocation(shaderProgram, "spotLight.quadratic"), spotLight.quadratic);

    // Material
    glUniform3f(glGetUniformLocation(shaderProgram, "material.ambient"), material.ambient.x, material.ambient.y, material.ambient.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.diffuse"), material.diffuse.x, material.diffuse.y, material.diffuse.z);
    glUniform3f(glGetUniformLocation(shaderProgram, "material.specular"), material.specular.x, material.specular.y, material.specular.z);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), material.shininess);
}