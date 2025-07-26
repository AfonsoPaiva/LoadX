#pragma once
#include <vector>
#include <string>
#include <map>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Mesh.h"

struct MaterialTextures {
    std::vector<Texture> diffuse;
    std::vector<Texture> specular;
    std::vector<Texture> normal;
    std::vector<Texture> height;
    std::vector<Texture> emission;
    std::vector<Texture> roughness;
    std::vector<Texture> metallic;
    std::vector<Texture> ao;
};

class Model {
public:
    Model(const std::string& path);
    void Draw(unsigned int shaderProgram);

    // Texture management
    void AddCustomTexture(const std::string& texturePath, const std::string& type);
    void ClearCustomTextures();
    MaterialTextures GetMaterialTextures() const { return customTextures; }

private:
    std::vector<Mesh> meshes;
    std::string directory;
    static std::vector<Texture> textures_loaded;
    MaterialTextures customTextures;

    void loadModel(const std::string& path);
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);
};