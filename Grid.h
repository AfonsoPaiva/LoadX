#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class Grid {
public:
    Grid();
    ~Grid();
    void Draw(unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection);

private:
    void setupMesh();

    unsigned int VAO, VBO, EBO;
    std::vector<glm::vec3> vertices;
    std::vector<unsigned int> indices;
};
