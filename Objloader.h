#pragma once
#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.h"

struct ObjMaterial {
    std::string name;
    glm::vec3 ambient = glm::vec3(0.2f);
    glm::vec3 diffuse = glm::vec3(0.8f);
    glm::vec3 specular = glm::vec3(1.0f);
    float shininess = 32.0f;
    std::string diffuseTexture;
    std::string normalTexture;
    std::string specularTexture;
};

class FastObjLoader {
public:
    static std::vector<Mesh> LoadOBJ(const std::string& objPath, const std::string& mtlPath = "");
    static std::vector<ObjMaterial> LoadMTL(const std::string& mtlPath);

    // Progress callback
    static void SetProgressCallback(std::function<void(float)> callback);

private:
    static std::vector<glm::vec3> positions;
    static std::vector<glm::vec2> texCoords;
    static std::vector<glm::vec3> normals;
    static std::vector<Vertex> vertices;
    static std::vector<unsigned int> indices;
    static std::vector<ObjMaterial> materials;
    static std::function<void(float)> progressCallback;

    static void parseLine(const std::string& line, size_t lineNumber, size_t totalLines);
    static void parseFace(const std::string& line);
    static Vertex getVertex(const std::string& vertexStr);
    static unsigned int TextureFromFile(const std::string& path, const std::string& directory);
    static void clear();
};