#include <glad/glad.h>
#include "mesh.h"
#include <iostream>
#include "materialprop.h" 

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures, MaterialProperties matProps)
    : vertices(vertices), indices(indices), textures(textures), materialProps(matProps) {
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
    // Set material properties as uniforms
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.ambient"), 1, &materialProps.ambient[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.diffuse"), 1, &materialProps.diffuse[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.specular"), 1, &materialProps.specular[0]);
    glUniform3fv(glGetUniformLocation(shaderProgram, "material.emission"), 1, &materialProps.emission[0]);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.shininess"), materialProps.shininess);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.opacity"), materialProps.opacity);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.roughness"), materialProps.roughness);
    glUniform1f(glGetUniformLocation(shaderProgram, "material.metallic"), materialProps.metallic);

    // Set texture availability flags (you'll need to implement this logic)
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasDiffuse"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasSpecular"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasNormal"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasHeight"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasEmission"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasRoughness"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasMetallic"), false);
    glUniform1i(glGetUniformLocation(shaderProgram, "material.hasAO"), false);

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

        if (name == "texture_diffuse") {
            number = std::to_string(diffuseNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasDiffuse"), true);
        }
        else if (name == "texture_specular") {
            number = std::to_string(specularNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasSpecular"), true);
        }
        else if (name == "texture_normal") {
            number = std::to_string(normalNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasNormal"), true);
        }
        else if (name == "texture_height") {
            number = std::to_string(heightNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasHeight"), true);
        }
        else if (name == "texture_emission") {
            number = std::to_string(emissionNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasEmission"), true);
        }
        else if (name == "texture_roughness") {
            number = std::to_string(roughnessNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasRoughness"), true);
        }
        else if (name == "texture_metallic") {
            number = std::to_string(metallicNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasMetallic"), true);
        }
        else if (name == "texture_ao") {
            number = std::to_string(aoNr++);
            glUniform1i(glGetUniformLocation(shaderProgram, "material.hasAO"), true);
        }

        // Set the sampler to the correct texture unit
        std::string uniformName = "material." + name + number;
        glUniform1i(glGetUniformLocation(shaderProgram, uniformName.c_str()), i);

        // Bind the texture
        glBindTexture(GL_TEXTURE_2D, textures[i].id);
    }

    // Draw mesh
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Always good practice to set everything back to defaults once configured.
    glActiveTexture(GL_TEXTURE0);
}