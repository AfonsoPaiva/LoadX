#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <glm/glm.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "materialprop.h" 
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
    std::vector<Texture> baseColor;
};

class Model {
public:
    Model(const std::string& path, const std::string& mtlPath = "");
    void Draw(unsigned int shaderProgram);

    // Texture management
    void AddCustomTexture(const std::string& texturePath, const std::string& type);
    void ClearCustomTextures();
    void LoadTexturesFromFolder(const std::string& folderPath);
    MaterialTextures GetMaterialTextures() const { return customTextures; }

    // MTL file handling
    void LoadMaterialFile(const std::string& mtlPath);
    bool IsObjFile() const { return isObjFile; }
    bool HasMtlFile() const { return hasMtlFile; }

    // Progress tracking
    bool IsLoading() const { return isLoading; }
    float GetLoadingProgress() const { return loadingProgress; }

    // UV debugging
    void FlipUVCoordinates();
    void SetUVFlipped(bool flipped) { uvFlipped = flipped; }
    bool IsUVFlipped() const { return uvFlipped; }

    // Model bounds and auto-sizing
    glm::vec3 GetModelCenter() const { return modelCenter; }
    glm::vec3 GetModelSize() const { return modelSize; }
    float GetRecommendedScale() const { return recommendedScale; }
    void CalculateModelBounds();

private:
    std::vector<Mesh> meshes;
    std::string directory;
    std::string modelPath;
    bool isObjFile;
    bool hasMtlFile;
    bool isLoading;
    bool uvFlipped;
    float loadingProgress;
    static std::vector<Texture> textures_loaded;
    MaterialTextures customTextures;

    // Model bounds for auto-sizing
    glm::vec3 minBounds;
    glm::vec3 maxBounds;
    glm::vec3 modelCenter;
    glm::vec3 modelSize;
    float recommendedScale;

    void loadModel(const std::string& path, const std::string& mtlPath = "");
    void processNode(aiNode* node, const aiScene* scene);
    Mesh processMesh(aiMesh* mesh, const aiScene* scene);
    std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);
    unsigned int TextureFromFile(const char* path, const std::string& directory, bool gamma = false);

    // Helper functions
    std::string getTextureTypeFromFilename(const std::string& filename);
    bool isImageFile(const std::string& filename);
    bool isObjFormat(const std::string& path);
    std::string getFileExtension(const std::string& path);

    //materail prop
    MaterialProperties extractMaterialProperties(aiMaterial* mat);
};