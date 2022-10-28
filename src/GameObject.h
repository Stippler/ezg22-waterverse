#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "AnimatedModel.h"
#include "Shader.h"

class GameObject
{
public:
    GameObject(AnimatedModel *model, bool animated = true);
    ~GameObject();

    glm::vec3 pos = glm::vec3(0.0f);

    float time = 0.0f;

    void render(Shader *shader);

    // void setPos(glm::vec3 pos);
    // void setRot(glm::vec3 rot);
private:
    glm::mat4 modelMatrix;
    AnimatedModel *model;
    bool animated = false;
};
