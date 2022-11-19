#pragma once

#include "Shader.h"

class GBuffer
{
public:
    unsigned int gBuffer;
    unsigned int gPosition, gNormal, gAlbedo;
    unsigned int gEnv, environment;
    unsigned int rboDepth;

    GBuffer();
    ~GBuffer();

    void resize();

    void render();
    void renderEnvironment();

private:
    bool reloadShader = false;

    Shader *shaderGeometryPass;
    Shader *envShader;
};