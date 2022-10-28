#pragma once

#include <vector>

#include "Shader.h"
#include "GBuffer.h"

class SSAO
{
public:
    unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

    SSAO(GBuffer *gbuffer);
    ~SSAO();

    void resize();
    void render();

private:
    GBuffer *gbuffer;

    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    bool reloadShader = false;
    Shader *shaderSSAO;
    Shader *shaderSSAOBlur;

    // ssao init
    unsigned int ssaoFBO, ssaoBlurFBO;
    std::vector<glm::vec3> ssaoKernel;
    unsigned int noiseTexture;

    void renderQuad();
    float ourLerp(float a, float b, float f);
};
