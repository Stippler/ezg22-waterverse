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
    Water(unsigned int width = 16, unsigned int height = 16);
    ~Water();

    void addDrop(glm::vec2 center, float radius, float strength);
    void stepSimulation();
    void updateNormals();

    void render();
private:

    unsigned int width, height;
    unsigned int VBO, VAO, EBO;

    unsigned int VBO2, VAO2, EBO2;

    unsigned int idx=0;

    glm::mat4 model;

    ComputeShader *test;
    ComputeShader *normal;
    ComputeShader *drop;
    ComputeShader *update;

    WaterTexture *texture;
    WaterTexture *copyTexture;

    Shader *waterShader;

    bool reloadCompute = false;
    bool reloadShader = false;

    void swapTexture();
};