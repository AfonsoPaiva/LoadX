#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>  
#include "model.h"
#include "Objloader.h"
#include "stb_image.h"
#include <iostream>
#include <filesystem>
#include "materialprop.h"
#include <algorithm>
#include <fstream>
#include <ios>
#include <chrono>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>  
#include <assimp/Importer.hpp>     

std::vector<Texture> Model::textures_loaded;

Model::Model(const std::string& path, const std::string& mtlPath)
    : modelPath(path), isObjFile(false), hasMtlFile(false), isLoading(true), loadingProgress(0.0f), uvFlipped(false),
    minBounds(FLT_MAX), maxBounds(-FLT_MAX), modelCenter(0.0f), modelSize(0.0f), recommendedScale(1.0f) {
    isObjFile = isObjFormat(path);

    try {
        loadModel(path, mtlPath);
        CalculateModelBounds();
        isLoading = false;
        loadingProgress = 1.0f;
    }
    catch (const std::exception& e) {
        std::cerr << "Failed to load model: " << e.what() << std::endl;
        isLoading = false;
        loadingProgress = 0.0f;
        throw;
    }
}


void Model::CalculateModelBounds() {
    if (meshes.empty()) {
        return;
    }

    // Reset bounds
    minBounds = glm::vec3(FLT_MAX);
    maxBounds = glm::vec3(-FLT_MAX);

    // Calculate bounds across all meshes
    for (const auto& mesh : meshes) {
        for (const auto& vertex : mesh.vertices) {
            minBounds.x = std::min(minBounds.x, vertex.Position.x);
            minBounds.y = std::min(minBounds.y, vertex.Position.y);
            minBounds.z = std::min(minBounds.z, vertex.Position.z);

            maxBounds.x = std::max(maxBounds.x, vertex.Position.x);
            maxBounds.y = std::max(maxBounds.y, vertex.Position.y);
            maxBounds.z = std::max(maxBounds.z, vertex.Position.z);
        }
    }

    // Calculate center and size
    modelCenter = (minBounds + maxBounds) * 0.5f;
    modelSize = maxBounds - minBounds;

    // Calculate recommended scale to fit in a reasonable viewport size
    float maxDimension = std::max({ modelSize.x, modelSize.y, modelSize.z });

    // Target size: models should fit in a 2-unit cube for reasonable viewing
    const float TARGET_SIZE = 2.0f;

    if (maxDimension > 0.0f) {
        recommendedScale = TARGET_SIZE / maxDimension;
    }
    else {
        recommendedScale = 1.0f;
    }

    std::cout << "Model bounds calculated:" << std::endl;
    std::cout << "  Min: (" << minBounds.x << ", " << minBounds.y << ", " << minBounds.z << ")" << std::endl;
    std::cout << "  Max: (" << maxBounds.x << ", " << maxBounds.y << ", " << maxBounds.z << ")" << std::endl;
    std::cout << "  Center: (" << modelCenter.x << ", " << modelCenter.y << ", " << modelCenter.z << ")" << std::endl;
    std::cout << "  Size: (" << modelSize.x << ", " << modelSize.y << ", " << modelSize.z << ")" << std::endl;
    std::cout << "  Recommended scale: " << recommendedScale << std::endl;
}

void Model::Draw(unsigned int shaderProgram) {
    for (unsigned int i = 0; i < meshes.size(); i++)
        meshes[i].Draw(shaderProgram);
}

bool Model::isObjFormat(const std::string& path) {
    std::string ext = getFileExtension(path);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    return ext == "obj";
}

std::string Model::getFileExtension(const std::string& path) {
    size_t lastDot = path.find_last_of(".");
    if (lastDot != std::string::npos) {
        return path.substr(lastDot + 1);
    }
    return "";
}

bool Model::isImageFile(const std::string& filename) {
    std::string ext = filename.substr(filename.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

    return ext == "png" || ext == "jpg" || ext == "jpeg" ||
        ext == "tga" || ext == "bmp" || ext == "hdr" ||
        ext == "dds" || ext == "tiff" || ext == "exr";
}

std::string Model::getTextureTypeFromFilename(const std::string& filename) {
    std::string lower = filename;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    // Remove file extension for pattern matching
    size_t lastDot = lower.find_last_of(".");
    if (lastDot != std::string::npos) {
        lower = lower.substr(0, lastDot);
    }

    // Check for common texture naming patterns
    if (lower.find("diffuse") != std::string::npos ||
        lower.find("diff") != std::string::npos ||
        lower.find("albedo") != std::string::npos ||
        lower.find("basecolor") != std::string::npos ||
        lower.find("base_color") != std::string::npos ||
        lower.find("color") != std::string::npos) {
        return "texture_diffuse";
    }

    if (lower.find("normal") != std::string::npos ||
        lower.find("norm") != std::string::npos ||
        lower.find("nrm") != std::string::npos) {
        return "texture_normal";
    }

    if (lower.find("specular") != std::string::npos ||
        lower.find("spec") != std::string::npos) {
        return "texture_specular";
    }

    if (lower.find("roughness") != std::string::npos ||
        lower.find("rough") != std::string::npos) {
        return "texture_roughness";
    }

    if (lower.find("metallic") != std::string::npos ||
        lower.find("metal") != std::string::npos ||
        lower.find("met") != std::string::npos) {
        return "texture_metallic";
    }

    if (lower.find("height") != std::string::npos ||
        lower.find("displacement") != std::string::npos ||
        lower.find("disp") != std::string::npos ||
        lower.find("bump") != std::string::npos) {
        return "texture_height";
    }

    if (lower.find("emission") != std::string::npos ||
        lower.find("emissive") != std::string::npos ||
        lower.find("emit") != std::string::npos ||
        lower.find("glow") != std::string::npos) {
        return "texture_emission";
    }

    if (lower.find("ao") != std::string::npos ||
        lower.find("ambient") != std::string::npos ||
        lower.find("occlusion") != std::string::npos) {
        return "texture_ao";
    }

    // If no specific pattern found, assume it's a diffuse/albedo texture
    return "texture_diffuse";
}

void Model::LoadMaterialFile(const std::string& mtlPath) {
    if (!isObjFile) {
        std::cout << "MTL files can only be loaded for OBJ models." << std::endl;
        return;
    }

    if (mtlPath.empty()) {
        std::cout << "No MTL file path provided." << std::endl;
        return;
    }

    // Reload the model with the MTL file
    meshes.clear();
    textures_loaded.clear();
    ClearCustomTextures();

    hasMtlFile = true;
    loadModel(modelPath, mtlPath);

    std::cout << "MTL file loaded: " << mtlPath << std::endl;
}

void Model::LoadTexturesFromFolder(const std::string& folderPath) {
    try {
        ClearCustomTextures();

        if (!std::filesystem::exists(folderPath) || !std::filesystem::is_directory(folderPath)) {
            std::cerr << "Invalid folder path: " << folderPath << std::endl;
            return;
        }

        std::cout << "Scanning folder for textures: " << folderPath << std::endl;

        int texturesFound = 0;
        for (const auto& entry : std::filesystem::directory_iterator(folderPath)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::string fullPath = entry.path().string();

                if (isImageFile(filename)) {
                    std::string textureType = getTextureTypeFromFilename(filename);

                    // Load the texture
                    Texture texture;
                    texture.id = TextureFromFile(fullPath.c_str(), "", false);
                    texture.type = textureType;
                    texture.path = fullPath;

                    if (texture.id != 0) {  // Only add if texture loaded successfully
                        // Add to appropriate category
                        if (textureType == "texture_diffuse") {
                            customTextures.diffuse.push_back(texture);
                            customTextures.baseColor.push_back(texture);  // Also add as baseColor
                        }
                        else if (textureType == "texture_specular") {
                            customTextures.specular.push_back(texture);
                        }
                        else if (textureType == "texture_normal") {
                            customTextures.normal.push_back(texture);
                        }
                        else if (textureType == "texture_height") {
                            customTextures.height.push_back(texture);
                        }
                        else if (textureType == "texture_emission") {
                            customTextures.emission.push_back(texture);
                        }
                        else if (textureType == "texture_roughness") {
                            customTextures.roughness.push_back(texture);
                        }
                        else if (textureType == "texture_metallic") {
                            customTextures.metallic.push_back(texture);
                        }
                        else if (textureType == "texture_ao") {
                            customTextures.ao.push_back(texture);
                        }

                        // Apply to all meshes
                        for (auto& mesh : meshes) {
                            mesh.textures.push_back(texture);
                        }

                        texturesFound++;
                        std::cout << "Loaded " << textureType << ": " << filename << std::endl;
                    }
                }
            }
        }

        std::cout << "Auto-loaded " << texturesFound << " textures from folder." << std::endl;

    }
    catch (const std::exception& e) {
        std::cerr << "Error loading textures from folder: " << e.what() << std::endl;
    }
}

void Model::AddCustomTexture(const std::string& texturePath, const std::string& type) {
    Texture texture;
    texture.id = TextureFromFile(texturePath.c_str(), "", false);
    texture.type = type;
    texture.path = texturePath;

    if (type == "texture_diffuse") {
        customTextures.diffuse.push_back(texture);
        customTextures.baseColor.push_back(texture);  // Also add as baseColor
    }
    else if (type == "texture_specular") {
        customTextures.specular.push_back(texture);
    }
    else if (type == "texture_normal") {
        customTextures.normal.push_back(texture);
    }
    else if (type == "texture_height") {
        customTextures.height.push_back(texture);
    }
    else if (type == "texture_emission") {
        customTextures.emission.push_back(texture);
    }
    else if (type == "texture_roughness") {
        customTextures.roughness.push_back(texture);
    }
    else if (type == "texture_metallic") {
        customTextures.metallic.push_back(texture);
    }
    else if (type == "texture_ao") {
        customTextures.ao.push_back(texture);
    }

    // Add custom textures to all meshes
    for (auto& mesh : meshes) {
        // Remove existing textures of this type
        mesh.textures.erase(
            std::remove_if(mesh.textures.begin(), mesh.textures.end(),
                [&type](const Texture& t) { return t.type == type; }),
            mesh.textures.end());

        // Add the new texture
        mesh.textures.push_back(texture);
    }
}

void Model::ClearCustomTextures() {
    customTextures.diffuse.clear();
    customTextures.specular.clear();
    customTextures.normal.clear();
    customTextures.height.clear();
    customTextures.emission.clear();
    customTextures.roughness.clear();
    customTextures.metallic.clear();
    customTextures.ao.clear();
    customTextures.baseColor.clear();
}

void Model::loadModel(const std::string& path, const std::string& mtlPath) {
    directory = path.substr(0, path.find_last_of('/'));

    if (isObjFile) {
        // Use fast OBJ loader
        std::cout << "Using Fast OBJ Loader for: " << path << std::endl;

        FastObjLoader::SetProgressCallback([this](float progress) {
            this->loadingProgress = progress;
            });

        try {
            meshes = FastObjLoader::LoadOBJ(path, mtlPath);

            if (!mtlPath.empty()) {
                hasMtlFile = true;
            }
            else {
                hasMtlFile = false;
            }

            if (meshes.empty()) {
                throw std::runtime_error("No meshes loaded from OBJ file");
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Fast OBJ loader failed: " << e.what() << std::endl;
            throw;
        }
    }
    else {
        // Use Assimp for other formats (including GLB/GLTF)
        std::cout << "Using Assimp for: " << path << std::endl;

        Assimp::Importer importer;

        // Base flags for all formats
        unsigned int flags = aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace;

        // Detect file format and apply appropriate flags
        std::string ext = getFileExtension(path);
        std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);

        if (ext == "gltf" || ext == "glb") {
            // GLTF/GLB
            std::cout << "Detected GLTF/GLB format - preserving original UV coordinates" << std::endl;
            flags |= aiProcess_ValidateDataStructure;
        }
        else if (ext == "fbx") {
            // FBX
            std::cout << "Detected FBX format - applying UV flip" << std::endl;
            flags |= aiProcess_FlipUVs | aiProcess_GlobalScale;
        }
        else if (ext == "dae") {
            // Collada
            std::cout << "Detected DAE format - applying UV flip" << std::endl;
            flags |= aiProcess_FlipUVs | aiProcess_FixInfacingNormals;
        }
        else if (ext == "3ds") {
            // 3DS
            std::cout << "Detected 3DS format - applying UV flip" << std::endl;
            flags |= aiProcess_FlipUVs | aiProcess_OptimizeMeshes;
        }
        else {
            // Default 
            std::cout << "Unknown format - applying default UV flip" << std::endl;
            flags |= aiProcess_FlipUVs;
        }

        flags |= aiProcess_JoinIdenticalVertices | aiProcess_OptimizeMeshes | aiProcess_RemoveRedundantMaterials;

        loadingProgress = 0.1f;
        const aiScene* scene = importer.ReadFile(path, flags);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
            std::cerr << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
            throw std::runtime_error("Failed to load model with Assimp: " + std::string(importer.GetErrorString()));
        }

        loadingProgress = 0.3f;

        if (scene->mNumTextures > 0) {
            std::cout << "Found " << scene->mNumTextures << " embedded textures" << std::endl;
        }

        loadingProgress = 0.5f;
        processNode(scene->mRootNode, scene);

        std::cout << "Successfully loaded " << meshes.size() << " meshes with proper UV coordinates" << std::endl;
    }
}



void Model::processNode(aiNode* node, const aiScene* scene) {
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        loadingProgress = (float)meshes.size() / (float)scene->mNumMeshes;

        try {
            meshes.push_back(processMesh(mesh, scene));
        }
        catch (const std::bad_alloc& e) {
            std::cerr << "Memory allocation failed for mesh " << i << ": " << e.what() << std::endl;
            continue;
        }
    }

    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene);
    }
}

MaterialProperties Model::extractMaterialProperties(aiMaterial* mat) {
    MaterialProperties props;

    aiString name;
    mat->Get(AI_MATKEY_NAME, name);
    props.name = name.C_Str();

    aiColor3D color;

    if (mat->Get(AI_MATKEY_BASE_COLOR, color) == AI_SUCCESS) {
        props.diffuse = glm::vec3(color.r, color.g, color.b);
        std::cout << "Using GLTF base color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
    }
    else if (mat->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        props.diffuse = glm::vec3(color.r, color.g, color.b);
        std::cout << "Using diffuse color: " << color.r << ", " << color.g << ", " << color.b << std::endl;
    }
    else {
        props.diffuse = glm::vec3(1.0f); 
        std::cout << "No color found, using default white" << std::endl;
    }

    if (mat->Get(AI_MATKEY_COLOR_AMBIENT, color) == AI_SUCCESS) {
        props.ambient = glm::vec3(color.r, color.g, color.b);
    }

    if (mat->Get(AI_MATKEY_COLOR_SPECULAR, color) == AI_SUCCESS) {
        props.specular = glm::vec3(color.r, color.g, color.b);
    }

    if (mat->Get(AI_MATKEY_COLOR_EMISSIVE, color) == AI_SUCCESS) {
        props.emission = glm::vec3(color.r, color.g, color.b);
    }

    float shininess;
    if (mat->Get(AI_MATKEY_SHININESS, shininess) == AI_SUCCESS) {
        props.shininess = shininess;
    }

    float opacity;
    if (mat->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
        props.opacity = opacity;
    }

    float roughness, metallic;
    if (mat->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) == AI_SUCCESS) {
        props.roughness = roughness;
        std::cout << "GLTF roughness: " << roughness << std::endl;
    }
    else {
        props.roughness = 0.5f; 
    }

    if (mat->Get(AI_MATKEY_METALLIC_FACTOR, metallic) == AI_SUCCESS) {
        props.metallic = metallic;
        std::cout << "GLTF metallic: " << metallic << std::endl;
    }
    else {
        props.metallic = 0.0f; 
    }

    std::cout << "Extracted material: " << props.name
        << " (Diffuse: " << props.diffuse.x << ", " << props.diffuse.y << ", " << props.diffuse.z
        << ", R:" << props.roughness << ", M:" << props.metallic << ")" << std::endl;

    return props;
}
Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene) {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;
    MaterialProperties matProps;

    vertices.reserve(mesh->mNumVertices);

    // Estimate indices count (triangulated mesh = 3 indices per face)
    unsigned int estimatedIndices = 0;
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        estimatedIndices += mesh->mFaces[i].mNumIndices;
    }
    indices.reserve(estimatedIndices);

    // Process vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;

        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        else {
            vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        if (mesh->mTextureCoords[0]) {
            glm::vec2 vec;
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;

            vertex.TexCoords = vec;
        }
        else {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        if (mesh->HasTangentsAndBitangents()) {
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;

            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else {
            vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
            vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    // Process material
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* mat = scene->mMaterials[mesh->mMaterialIndex];
        matProps = extractMaterialProperties(mat);


        // 1. DIFFUSE/BASE COLOR 
        std::vector<Texture> diffuseMaps;

        // Try GLTF base color first
        diffuseMaps = loadMaterialTextures(mat, aiTextureType_BASE_COLOR, "texture_diffuse");
        if (diffuseMaps.empty()) {
            // Try standard diffuse
            diffuseMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE, "texture_diffuse");
        }
        if (diffuseMaps.empty()) {
            // Try alternative diffuse types for other formats
            diffuseMaps = loadMaterialTextures(mat, aiTextureType_UNKNOWN, "texture_diffuse");
        }
        textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

        // 2. NORMAL MAPS
        std::vector<Texture> normalMaps = loadMaterialTextures(mat, aiTextureType_NORMALS, "texture_normal");
        if (normalMaps.empty()) {
            normalMaps = loadMaterialTextures(mat, aiTextureType_HEIGHT, "texture_normal");
        }
        if (normalMaps.empty()) {
            normalMaps = loadMaterialTextures(mat, aiTextureType_DISPLACEMENT, "texture_normal");
        }
        textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

        // 3. SPECULAR MAPS
        std::vector<Texture> specularMaps = loadMaterialTextures(mat, aiTextureType_SPECULAR, "texture_specular");
        textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

        // 4. PBR TEXTURES

        // Roughness
        std::vector<Texture> roughnessMaps = loadMaterialTextures(mat, aiTextureType_DIFFUSE_ROUGHNESS, "texture_roughness");
        if (roughnessMaps.empty()) {
            roughnessMaps = loadMaterialTextures(mat, aiTextureType_SHININESS, "texture_roughness");
        }
        textures.insert(textures.end(), roughnessMaps.begin(), roughnessMaps.end());

        // Metallic
        std::vector<Texture> metallicMaps = loadMaterialTextures(mat, aiTextureType_METALNESS, "texture_metallic");
        if (metallicMaps.empty()) {
            metallicMaps = loadMaterialTextures(mat, aiTextureType_REFLECTION, "texture_metallic");
        }
        textures.insert(textures.end(), metallicMaps.begin(), metallicMaps.end());

        // Emission
        std::vector<Texture> emissionMaps = loadMaterialTextures(mat, aiTextureType_EMISSIVE, "texture_emission");
        if (emissionMaps.empty()) {
            emissionMaps = loadMaterialTextures(mat, aiTextureType_UNKNOWN, "texture_emission");
        }
        textures.insert(textures.end(), emissionMaps.begin(), emissionMaps.end());

        // Ambient Occlusion
        std::vector<Texture> aoMaps = loadMaterialTextures(mat, aiTextureType_AMBIENT_OCCLUSION, "texture_ao");
        if (aoMaps.empty()) {
            aoMaps = loadMaterialTextures(mat, aiTextureType_LIGHTMAP, "texture_ao");
        }
        textures.insert(textures.end(), aoMaps.begin(), aoMaps.end());
    }

    return Mesh(vertices, indices, textures, matProps);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName) {
    std::vector<Texture> textures;

    std::cout << "Loading textures of type: " << typeName << " (aiTextureType: " << type << ")" << std::endl;
    std::cout << "Texture count for this type: " << mat->GetTextureCount(type) << std::endl;

    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::cout << "Found texture path: " << str.C_Str() << std::endl;

        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++) {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
                textures.push_back(textures_loaded[j]);
                skip = true;
                std::cout << "Using cached texture: " << str.C_Str() << std::endl;
                break;
            }
        }

        if (!skip) {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();

            if (texture.id != 0) {
                textures.push_back(texture);
                textures_loaded.push_back(texture);
                std::cout << "Successfully loaded new texture: " << str.C_Str() << " with ID: " << texture.id << std::endl;
            }
            else {
                std::cout << "Failed to load texture: " << str.C_Str() << std::endl;
            }
        }
    }

    std::cout << "Total textures loaded for type " << typeName << ": " << textures.size() << std::endl;
    return textures;
}

unsigned int Model::TextureFromFile(const char* path, const std::string& directory, bool gamma) {
    std::string filename = std::string(path);

    if (filename[0] == '*') {
        int textureIndex = std::stoi(filename.substr(1));
        std::cout << "Loading embedded texture index: " << textureIndex << std::endl;
        return 0;
    }

    if (!directory.empty()) {
        filename = directory + '/' + filename;
    }

    std::cout << "Attempting to load texture from: " << filename << std::endl;

    // Check if file exists
    std::ifstream file(filename);
    if (!file.good()) {
        std::cout << "Texture file not found: " << filename << std::endl;

        // Try different extensions for missing textures
        std::vector<std::string> extensions = { ".png", ".jpg", ".jpeg", ".tga", ".bmp" };
        std::string baseName = filename.substr(0, filename.find_last_of('.'));

        for (const auto& ext : extensions) {
            std::string altPath = baseName + ext;
            std::ifstream altFile(altPath);
            if (altFile.good()) {
                altFile.close();
                filename = altPath;
                std::cout << "Found alternative texture: " << filename << std::endl;
                break;
            }
            altFile.close();
        }

        file.close();

        std::ifstream finalCheck(filename);
        if (!finalCheck.good()) {
            finalCheck.close();
            return 0;
        }
        finalCheck.close();
    }
    file.close();

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;

    // Force 4 components for better compatibility
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        std::cout << "Texture loaded successfully: " << filename << std::endl;
        std::cout << "  Dimensions: " << width << "x" << height << std::endl;
        std::cout << "  Components: " << nrComponents << std::endl;

        GLenum format;
        GLenum internalFormat;

        if (nrComponents == 1) {
            format = GL_RED;
            internalFormat = GL_RED;
        }
        else if (nrComponents == 3) {
            format = GL_RGB;
            internalFormat = gamma ? GL_SRGB : GL_RGB;
        }
        else if (nrComponents == 4) {
            format = GL_RGBA;
            internalFormat = gamma ? GL_SRGB_ALPHA : GL_RGBA;
        }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


        stbi_image_free(data);

        std::cout << "Texture bound with ID: " << textureID << std::endl;
    }
    else {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        std::cout << "STB Error: " << stbi_failure_reason() << std::endl;
        stbi_image_free(data);
        return 0;
    }

    return textureID;
}

void Model::FlipUVCoordinates() {
    for (auto& mesh : meshes) {
        for (auto& vertex : mesh.vertices) {
            vertex.TexCoords.y = 1.0f - vertex.TexCoords.y;
        }

        mesh.setupMesh();
    }
    uvFlipped = !uvFlipped;

    std::cout << "UV coordinates " << (uvFlipped ? "flipped" : "restored to original") << std::endl;
}