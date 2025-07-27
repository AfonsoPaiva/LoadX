#pragma once
#include <glm/glm.hpp>
#include <string>

struct MaterialProperties {
    std::string name;
    glm::vec3 ambient = glm::vec3(0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(1.0f);
    glm::vec3 emission = glm::vec3(0.0f);
    float shininess = 32.0f;
    float opacity = 1.0f;
    float roughness = 0.5f;
    float metallic = 0.0f;
};