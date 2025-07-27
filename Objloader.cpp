#include "Objloader.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <glad/glad.h>
#include "stb_image.h"
#include <map>

// Static member definitions
std::vector<glm::vec3> FastObjLoader::positions;
std::vector<glm::vec2> FastObjLoader::texCoords;
std::vector<glm::vec3> FastObjLoader::normals;
std::vector<Vertex> FastObjLoader::vertices;
std::vector<unsigned int> FastObjLoader::indices;
std::vector<ObjMaterial> FastObjLoader::materials;
std::function<void(float)> FastObjLoader::progressCallback;

void FastObjLoader::SetProgressCallback(std::function<void(float)> callback) {
    progressCallback = callback;
}

std::vector<Texture> FastObjLoader::loadTexturesForMaterial(const std::string& materialName, const std::string& directory) {
    std::vector<Texture> textures;

    // Find the material
    ObjMaterial* mat = nullptr;
    for (auto& material : materials) {
        if (material.name == materialName) {
            mat = &material;
            break;
        }
    }

    if (!mat) {
        return textures; // Return empty if material not found
    }

    // Load diffuse texture
    if (!mat->diffuseTexture.empty()) {
        Texture texture;
        texture.id = TextureFromFile(mat->diffuseTexture, directory);
        texture.type = "texture_diffuse";
        texture.path = mat->diffuseTexture;
        if (texture.id != 0) {
            textures.push_back(texture);
            std::cout << "Loaded diffuse texture: " << mat->diffuseTexture << std::endl;
        }
    }

    // Load normal texture
    if (!mat->normalTexture.empty()) {
        Texture texture;
        texture.id = TextureFromFile(mat->normalTexture, directory);
        texture.type = "texture_normal";
        texture.path = mat->normalTexture;
        if (texture.id != 0) {
            textures.push_back(texture);
            std::cout << "Loaded normal texture: " << mat->normalTexture << std::endl;
        }
    }

    // Load specular texture
    if (!mat->specularTexture.empty()) {
        Texture texture;
        texture.id = TextureFromFile(mat->specularTexture, directory);
        texture.type = "texture_specular";
        texture.path = mat->specularTexture;
        if (texture.id != 0) {
            textures.push_back(texture);
            std::cout << "Loaded specular texture: " << mat->specularTexture << std::endl;
        }
    }

    return textures;
}

std::vector<Mesh> FastObjLoader::LoadOBJ(const std::string& objPath, const std::string& mtlPath) {
    auto start = std::chrono::high_resolution_clock::now();

    clear();

    // Load materials if MTL file is provided
    if (!mtlPath.empty()) {
        materials = LoadMTL(mtlPath);
        std::cout << "Loaded " << materials.size() << " materials from MTL file" << std::endl;
    }

    std::ifstream file(objPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open OBJ file: " << objPath << std::endl;
        return {};
    }

    // Get file size for progress tracking
    file.seekg(0, std::ios::end);
    size_t fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // Reserve memory (rough estimates)
    positions.reserve(fileSize / 50);
    texCoords.reserve(fileSize / 60);
    normals.reserve(fileSize / 50);
    vertices.reserve(fileSize / 40);
    indices.reserve(fileSize / 15);

    std::cout << "Loading OBJ file: " << objPath << " (" << (fileSize / (1024 * 1024)) << " MB)" << std::endl;

    std::string line;
    size_t lineNumber = 0;
    size_t processedBytes = 0;
    std::string currentMaterial = "";
    std::vector<Mesh> meshes;

    // Track per-material geometry
    std::map<std::string, std::vector<Vertex>> materialVertices;
    std::map<std::string, std::vector<unsigned int>> materialIndices;
    std::map<std::string, std::unordered_map<std::string, unsigned int>> materialVertexCache;

    while (std::getline(file, line)) {
        lineNumber++;
        processedBytes += line.length() + 1;

        // Update progress every 10000 lines
        if (lineNumber % 10000 == 0) {
            float progress = (float)processedBytes / (float)fileSize;
            if (progressCallback) {
                progressCallback(progress * 0.8f);
            }
        }

        if (line.empty() || line[0] == '#') continue;

        // Handle material usage
        if (line.substr(0, 6) == "usemtl") {
            std::istringstream iss(line);
            std::string command;
            iss >> command >> currentMaterial;
            std::cout << "Using material: " << currentMaterial << std::endl;
            continue;
        }

        // Parse geometry with current material context
        if (line[0] == 'f' && line[1] == ' ') {
            parseFaceWithMaterial(line, currentMaterial, materialVertices, materialIndices, materialVertexCache);
        }
        else {
            parseLine(line, lineNumber, fileSize);
        }
    }

    file.close();

    if (progressCallback) {
        progressCallback(0.9f);
    }

    // Create meshes for each material
    std::string directory = objPath.substr(0, objPath.find_last_of('/'));
    if (directory == objPath) directory = objPath.substr(0, objPath.find_last_of('\\'));
    if (directory == objPath) directory = "";

    for (const auto& materialPair : materialVertices) {
        const std::string& matName = materialPair.first;
        const std::vector<Vertex>& verts = materialPair.second;
        const auto& indicesIt = materialIndices.find(matName);

        if (indicesIt != materialIndices.end()) {
            const std::vector<unsigned int>& inds = indicesIt->second;

            if (!verts.empty() && !inds.empty()) {
                std::vector<Texture> textures = loadTexturesForMaterial(matName, directory);
                meshes.emplace_back(verts, inds, textures);
                std::cout << "Created mesh for material '" << matName << "' with " << verts.size() << " vertices" << std::endl;
            }
        }
    }

    // If no materials were used, create a single mesh with all geometry
    if (materialVertices.empty() && !vertices.empty() && !indices.empty()) {
        std::vector<Texture> textures;
        meshes.emplace_back(vertices, indices, textures);
        std::cout << "Created single mesh without materials" << std::endl;
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    std::cout << "Fast OBJ loading completed in " << duration.count() << "ms" << std::endl;
    std::cout << "Created " << meshes.size() << " mesh(es)" << std::endl;

    if (progressCallback) {
        progressCallback(1.0f);
    }

    return meshes;
}

void FastObjLoader::parseLine(const std::string& line, size_t lineNumber, size_t totalLines) {
    if (line.length() < 2) return;

    const char* data = line.c_str();

    switch (data[0]) {
    case 'v':
        if (data[1] == ' ') {
            // Vertex position
            float x, y, z;
            if (sscanf_s(data + 2, "%f %f %f", &x, &y, &z) == 3) {
                positions.emplace_back(x, y, z);
            }
        }
        else if (data[1] == 't' && data[2] == ' ') {
            // Texture coordinate
            float u, v;
            if (sscanf_s(data + 3, "%f %f", &u, &v) >= 2) {
                texCoords.emplace_back(u, 1.0f - v);
            }
        }
        else if (data[1] == 'n' && data[2] == ' ') {
            // Normal
            float x, y, z;
            if (sscanf_s(data + 3, "%f %f %f", &x, &y, &z) == 3) {
                normals.emplace_back(x, y, z);
            }
        }
        break;

    case 'f':
        if (data[1] == ' ') {
            parseFace(line);
        }
        break;
    }
}

void FastObjLoader::parseFace(const std::string& line) {
    static std::unordered_map<std::string, unsigned int> vertexCache;

    std::istringstream iss(line.substr(2)); // Skip "f "
    std::string vertexStr;
    std::vector<unsigned int> faceIndices;

    while (iss >> vertexStr) {
        // Check cache first
        auto it = vertexCache.find(vertexStr);
        if (it != vertexCache.end()) {
            faceIndices.push_back(it->second);
        }
        else {
            // Create new vertex
            Vertex vertex = getVertex(vertexStr);
            unsigned int index = vertices.size();
            vertices.push_back(vertex);
            vertexCache[vertexStr] = index;
            faceIndices.push_back(index);
        }
    }

    // Triangulate face (fan triangulation for polygons with more than 3 vertices)
    for (size_t i = 1; i < faceIndices.size() - 1; i++) {
        indices.push_back(faceIndices[0]);
        indices.push_back(faceIndices[i]);
        indices.push_back(faceIndices[i + 1]);
    }
}

void FastObjLoader::parseFaceWithMaterial(const std::string& line, const std::string& material,
    std::map<std::string, std::vector<Vertex>>& materialVertices,
    std::map<std::string, std::vector<unsigned int>>& materialIndices,
    std::map<std::string, std::unordered_map<std::string, unsigned int>>& materialVertexCache) {

    std::string matKey = material.empty() ? "default" : material;
    auto& vertexCache = materialVertexCache[matKey];
    auto& verts = materialVertices[matKey];
    auto& inds = materialIndices[matKey];

    std::istringstream iss(line.substr(2)); // Skip "f "
    std::string vertexStr;
    std::vector<unsigned int> faceIndices;

    while (iss >> vertexStr) {
        // Check cache first
        auto it = vertexCache.find(vertexStr);
        if (it != vertexCache.end()) {
            faceIndices.push_back(it->second);
        }
        else {
            // Create new vertex
            Vertex vertex = getVertex(vertexStr);
            unsigned int index = static_cast<unsigned int>(verts.size());
            verts.push_back(vertex);
            vertexCache[vertexStr] = index;
            faceIndices.push_back(index);
        }
    }

    // Triangulate face
    for (size_t i = 1; i < faceIndices.size() - 1; i++) {
        inds.push_back(faceIndices[0]);
        inds.push_back(faceIndices[i]);
        inds.push_back(faceIndices[i + 1]);
    }
}

Vertex FastObjLoader::getVertex(const std::string& vertexStr) {
    Vertex vertex;

    // Initialize defaults
    vertex.Position = glm::vec3(0.0f);
    vertex.Normal = glm::vec3(0.0f, 1.0f, 0.0f);
    vertex.TexCoords = glm::vec2(0.0f);
    vertex.Tangent = glm::vec3(1.0f, 0.0f, 0.0f);
    vertex.Bitangent = glm::vec3(0.0f, 1.0f, 0.0f);

    // Parse vertex string (format: pos/tex/norm or pos//norm or pos/tex or just pos)
    int posIndex = -1, texIndex = -1, normIndex = -1;

    const char* str = vertexStr.c_str();
    char* endptr;

    // Parse position index
    posIndex = strtol(str, &endptr, 10);
    if (posIndex > 0 && posIndex <= (int)positions.size()) {
        vertex.Position = positions[posIndex - 1];
    }

    if (*endptr == '/') {
        str = endptr + 1;

        // Parse texture coordinate index
        if (*str != '/') {
            texIndex = strtol(str, &endptr, 10);
            if (texIndex > 0 && texIndex <= (int)texCoords.size()) {
                vertex.TexCoords = texCoords[texIndex - 1];
            }
        }

        if (*endptr == '/') {
            str = endptr + 1;

            // Parse normal index
            normIndex = strtol(str, &endptr, 10);
            if (normIndex > 0 && normIndex <= (int)normals.size()) {
                vertex.Normal = normals[normIndex - 1];
            }
        }
    }

    return vertex;
}

std::vector<ObjMaterial> FastObjLoader::LoadMTL(const std::string& mtlPath) {
    std::vector<ObjMaterial> materials;
    std::ifstream file(mtlPath);

    if (!file.is_open()) {
        std::cerr << "Failed to open MTL file: " << mtlPath << std::endl;
        return materials;
    }

    ObjMaterial currentMaterial;
    std::string line;
    bool hasMaterial = false;

    std::cout << "Loading MTL file: " << mtlPath << std::endl;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;

        std::istringstream iss(line);
        std::string command;
        iss >> command;

        if (command == "newmtl") {
            if (hasMaterial) {
                materials.push_back(currentMaterial);
                std::cout << "Loaded material: " << currentMaterial.name << std::endl;
            }

            currentMaterial = ObjMaterial(); // Reset to defaults
            iss >> currentMaterial.name;
            hasMaterial = true;
        }
        // Standard MTL properties
        else if (command == "Ka") { // Ambient
            iss >> currentMaterial.ambient.x >> currentMaterial.ambient.y >> currentMaterial.ambient.z;
        }
        else if (command == "Kd") { // Diffuse
            iss >> currentMaterial.diffuse.x >> currentMaterial.diffuse.y >> currentMaterial.diffuse.z;
        }
        else if (command == "Ks") { // Specular
            iss >> currentMaterial.specular.x >> currentMaterial.specular.y >> currentMaterial.specular.z;
        }
        else if (command == "Ke") { // Emission
            iss >> currentMaterial.emission.x >> currentMaterial.emission.y >> currentMaterial.emission.z;
        }
        else if (command == "Ns") { // Shininess
            iss >> currentMaterial.shininess;
        }
        else if (command == "d" || command == "Tr") { // Opacity
            iss >> currentMaterial.opacity;
        }
        else if (command == "Ni") { // Refraction index
            iss >> currentMaterial.refraction;
        }
        // PBR Extensions
        else if (command == "Pr") { // Roughness
            iss >> currentMaterial.roughness;
        }
        else if (command == "Pm") { // Metallic
            iss >> currentMaterial.metallic;
        }
        // Texture maps
        else if (command == "map_Kd") {
            std::getline(iss, currentMaterial.diffuseTexture);
            currentMaterial.diffuseTexture = currentMaterial.diffuseTexture.substr(1); // Remove leading space
        }
        else if (command == "map_Ks") {
            std::getline(iss, currentMaterial.specularTexture);
            currentMaterial.specularTexture = currentMaterial.specularTexture.substr(1);
        }
        else if (command == "map_Bump" || command == "bump") {
            std::getline(iss, currentMaterial.normalTexture);
            currentMaterial.normalTexture = currentMaterial.normalTexture.substr(1);
        }
        else if (command == "map_Disp") {
            std::getline(iss, currentMaterial.heightTexture);
            currentMaterial.heightTexture = currentMaterial.heightTexture.substr(1);
        }
        else if (command == "map_Ke") {
            std::getline(iss, currentMaterial.emissionTexture);
            currentMaterial.emissionTexture = currentMaterial.emissionTexture.substr(1);
        }
        else if (command == "map_Pr") {
            std::getline(iss, currentMaterial.roughnessTexture);
            currentMaterial.roughnessTexture = currentMaterial.roughnessTexture.substr(1);
        }
        else if (command == "map_Pm") {
            std::getline(iss, currentMaterial.metallicTexture);
            currentMaterial.metallicTexture = currentMaterial.metallicTexture.substr(1);
        }
        else if (command == "map_Ao") {
            std::getline(iss, currentMaterial.aoTexture);
            currentMaterial.aoTexture = currentMaterial.aoTexture.substr(1);
        }
        else if (command == "map_d") {
            std::getline(iss, currentMaterial.opacityTexture);
            currentMaterial.opacityTexture = currentMaterial.opacityTexture.substr(1);
        }
    }

    // Don't forget the last material
    if (hasMaterial) {
        materials.push_back(currentMaterial);
        std::cout << "Loaded material: " << currentMaterial.name << std::endl;
    }

    std::cout << "Total materials loaded: " << materials.size() << std::endl;
    return materials;
}

unsigned int FastObjLoader::TextureFromFile(const std::string& path, const std::string& directory) {
    std::string filename = directory.empty() ? path : directory + "/" + path;

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data) {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else {
        std::cerr << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

void FastObjLoader::clear() {
    positions.clear();
    texCoords.clear();
    normals.clear();
    vertices.clear();
    indices.clear();
    materials.clear();
}