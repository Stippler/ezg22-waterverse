#pragma once

#include <vector>
#include <glm/glm.hpp>

#include "GameObject.h"
#include "Shader.h"

namespace World
{
    void init();
    void free();

    GameObject *addGameObject(std::string model, bool animated = false, glm::vec3 pos = glm::vec3(0.0f), float scale = 1.0f);

    void update(float tslf);
    void renderGameObjects(Shader *shader);
    void renderSpheres(Shader *shader);
    void renderWater(Shader *shader);
    void renderCaustics(unsigned int environment);
    unsigned int getCaustics();

    // void renderWater(Shader *shader);

    glm::mat4 getLightSpaceMatrix();

    std::vector<GameObject *> getAnimatedObjects();
    std::vector<GameObject *> getStaticObjects();
    std::vector<GameObject *> getSpheres();

    std::vector<PointLight *> getPointLight();
    DirLight *getDirLight();
    void reload();
    void clear();
    float getTime();
}
