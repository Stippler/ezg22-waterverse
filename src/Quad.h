#pragma once

#include "Shader.h"

class Quad
{
public:
    Quad();
    ~Quad();

    void render(Shader *shader);

private:
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
};