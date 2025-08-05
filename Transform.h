#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Transform {
public:
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;

    Transform() :
        position(0.0f, 0.0f, 0.0f),
        rotation(0.0f, 0.0f, 0.0f),
        scale(1.0f, 1.0f, 1.0f) {
    }

    glm::mat4 GetModelMatrix() const {
        glm::mat4 model = glm::mat4(1.0f);

        //Scale -> Rotate -> Translate
        model = glm::translate(model, position);

        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, scale);

        return model;
    }

    // Helper method to get model matrix with center offset
    glm::mat4 GetModelMatrix(const glm::vec3& modelCenter) const {
        glm::mat4 model = glm::mat4(1.0f);

        //Translate to position -> Rotate -> Scale -> Center offset
        model = glm::translate(model, position);

        model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));

        model = glm::scale(model, scale);

        //Translating by negative model center
        model = glm::translate(model, -modelCenter);

        return model;
    }
};
