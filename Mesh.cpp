#include <glad/glad.h>
#include "mesh.h"
#include <iostream>

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : vertices(vertices), indices(indices), textures(textures) {
    setupMesh();
}

void Mesh::setupMesh() {
    // Create buffers/arrays
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Load data into vertex buffers
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    // Set the vertex attribute pointers
    // Vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // Vertex Normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));

    // Vertex Texture Coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));

    // Vertex Tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));

    // Vertex Bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

    glBindVertexArray(0);
}

void Mesh::Draw(unsigned int shaderProgram) {
    // Bind appropriate textures
    unsigned int diffuseNr = 1;
    unsigned int specularNr = 1;
    unsigned int normalNr = 1;
    unsigned int heightNr = 1;
    unsigned int emissionNr = 1;
    unsigned int roughnessNr = 1;
    unsigned int metallicNr = 1;
    unsigned int aoNr = 1;

    for (unsigned int i = 0; i < textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);

        std::string number;
        std::string name = textures[i].type;

        if (name == "texture_diffuse")
            number = std::to_string(diffuseNr++);
        else if (name == "texture_specular")
            number = std::to_string(specularNr++);
        else if (name == "texture_normal")
            number = std::to_string(normalNr++);
        else if (name == "texture_height")
            number = std::to_string(heightNr++);
        else if (name == "texture_emission")
            number = std::to_string(emissionNr++);
        else if (name == "texture_roughness")
            number = std::to_string(roughnessNr++);
        else if (name == "texture_metallic")
            number = std::to_string(metallicNr++);
        else if (name == "texture_ao")
            number = std::to_string(aoNr++);

        // Set the sampler to the correct texture unit
        std::string uniformName = "material." + name + number;
        glUniform1i(glGetUniformLocation(shaderProgram, uniformName.c_str()), i);

        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // Set texture availability flags
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasDiffuse"), diffuseNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasSpecular"), specularNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasNormal"), normalNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasHeight"), heightNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasEmission"), emissionNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasRoughness"), roughnessNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasMetallic"), metallicNr > 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasAO"), aoNr > 1);

    // Draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}