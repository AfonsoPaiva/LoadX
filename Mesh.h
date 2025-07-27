#pragma once
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "materialprop.h"

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    unsigned int id;
    std::string type;
    std::string path;
};

class Mesh {
public:
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    MaterialProperties materialProps;

    unsigned int VAO;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, MaterialProperties matProps = MaterialProperties{});
    void Draw(unsigned int shaderProgram);

private:
    unsigned int VBO, EBO;
    void setupMesh();
};