#pragma once

#include "ComputeShader.h"
#include "Shader.h"

class Water
{
public:
    Water(unsigned int width = 512, unsigned int height = 512);

    void render();
private:
    unsigned int width, height;

    unsigned int VBO, VAO, EBO;
    unsigned int texture;

    ComputeShader *test;
    Shader *waterShader;

    bool reloadCompute = false;
    bool reloadShader = false;

    void createTexture(unsigned int &texture);
};