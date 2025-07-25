#pragma once
#include <glm/glm.hpp>

struct DirectionalLight {
    bool enabled = true;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};

struct PointLight {
    bool enabled = true;
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct SpotLight {
    bool enabled = true;
    glm::vec3 position;
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float cutOff;
    float outerCutOff;

    float constant;
    float linear;
    float quadratic;
};

struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
};