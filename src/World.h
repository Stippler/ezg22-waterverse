#pragma once

#include <vector>
#include <glm/glm.hpp>
#include "GameObject.h"
#include "Shader.h"

namespace World
{
    void init();
    void free();

    GameObject* addGameObject(std::string model, glm::vec3 pos=glm::vec3(0.0f));

    void update(float tslf);
    void render(Shader *shader);

    glm::mat4 getLightSpaceMatrix();

    std::vector<GameObject*> getAnimatedObjects();
    std::vector<GameObject*> getStaticObjects();

    PointLight *getPointLight();
    DirLight *getDirLight();
}
