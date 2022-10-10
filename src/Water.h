#pragma once

#include "ComputeShader.h"
#include "Shader.h"
#include "WaterTexture.h"

class Water
{
public:
    Water(unsigned int width = 512, unsigned int height = 512);
    ~Water();

    void render();
private:
    unsigned int width, height;

    unsigned int VBO, VAO, EBO;

    ComputeShader *test;
    Shader *waterShader;
    WaterTexture *texture;

    bool reloadCompute = false;
    bool reloadShader = false;
};