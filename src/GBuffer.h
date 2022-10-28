#pragma once

#include "Shader.h"

class GBuffer
{
public:
    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedo;
    unsigned int rboDepth;

    GBuffer();
    ~GBuffer();

    void resize();

    void render();

private:
    bool reloadShader = false;

    Shader *shaderGeometryPass;
};