
#include "Grid.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

Grid::Grid() {
    // Create a simple large quad instead of grid lines
    vertices = {
        glm::vec3(-1000.0f, 0.0f, -1000.0f),
        glm::vec3(1000.0f, 0.0f, -1000.0f),
        glm::vec3(1000.0f, 0.0f,  1000.0f),
        glm::vec3(-1000.0f, 0.0f,  1000.0f)
    };

    // Two triangles to form a quad
    indices = {
        0, 1, 2,
        2, 3, 0
    };

    setupMesh();
}

Grid::~Grid() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}

void Grid::setupMesh() {
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

void Grid::Draw(unsigned int shaderProgram, glm::mat4 view, glm::mat4 projection) {
    glUseProgram(shaderProgram);

    // Enable blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set uniforms
    glm::vec3 gridColor(0.5f, 0.5f, 0.5f);
    unsigned int colorLoc = glGetUniformLocation(shaderProgram, "gridColor");
    glUniform3fv(colorLoc, 1, glm::value_ptr(gridColor));

    unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glDisable(GL_BLEND);
}