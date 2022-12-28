#pragma once

#include "Shader.h"

class WaterGBuffer
{
public:
    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedo;
    unsigned int rboDepth;

    WaterGBuffer();
    ~WaterGBuffer();

    void resize();

    void render();

private:
    bool reloadShader = false;

    Shader *shaderGeometryPass;
};