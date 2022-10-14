#pragma once

#include <glm/glm.hpp>

#include "ComputeShader.h"
#include "Shader.h"
#include "WaterTexture.h"

struct GridVertex{
    glm::vec3 pos;
    glm::vec2 texCoords;
};

class Water
{
public:
    Water(unsigned int width = 512, unsigned int height = 512);
    ~Water();

    void render();
private:

    unsigned int width, height;
    unsigned int VBO, VAO, EBO;

    unsigned int idx=0;

    glm::mat4 model;

    ComputeShader *test;
    Shader *waterShader;
    WaterTexture *texture;

    bool reloadCompute = false;
    bool reloadShader = false;
};