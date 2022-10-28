#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "Window.h"
#include "Shader.h"
#include "FileWatcher.h"
#include "Cube.h"
#include "Water.h"
#include "AnimatedModel.h"
#include "ShadowMap.h"
#include "Model.h"
#include "DirLight.h"
#include "PointLight.h"
#include "Material.h"
#include "GBuffer.h"
#include "World.h"

#include <random>

// Water
Water *water;
GBuffer *gbuffer;

bool reloadShader = false;

unsigned int modelLoc, viewLoc, projectionLoc;

// init depth shader
Shader *depthSkinning;
Shader *skinningShader;

Shader *shaderGeometryPass;
Shader *shaderSSAO;
// Shader *shaderLightingPass;
Shader *shaderSSAOBlur;

// Shadow predefines
ShadowMap *shadowMap;

// lights predefines
DirLight *light;
PointLight *plight;
Material *coral;

// animation predefines
long long StartTimeMillis = 0;

// ssao init
unsigned int ssaoFBO, ssaoBlurFBO;
std::vector<glm::vec3> ssaoKernel;
unsigned int noiseTexture;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;

float ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}

void Renderer::init()
{
    glEnable(GL_DEPTH_TEST);

    water = new Water();

    const char *vertexShader = "assets/shaders/depthSkinning.vert";
    const char *fragmentShader = "assets/shaders/depth.frag";
    depthSkinning = new Shader(vertexShader, fragmentShader);

    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });


    const char *vertexShader5 = "assets/shaders/skinning.vert";
    const char *fragmentShader5 = "assets/shaders/skinning.frag";
    skinningShader = new Shader(vertexShader5, fragmentShader5);
    FileWatcher::add(vertexShader5, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader5, []()
                     { reloadShader = true; });

    const char *vertexShader7 = "assets/shaders/ssao.vert";
    const char *fragmentShader7 = "assets/shaders/ssao.frag";
    shaderSSAO = new Shader(vertexShader7, fragmentShader7);
    FileWatcher::add(vertexShader7, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader7, []()
                     { reloadShader = true; });

    // const char *vertexShader8 = "assets/shaders/ssao.vert";
    // const char *fragmentShader8 = "assets/shaders/ssao_lighting.frag";
    // shaderLightingPass = new Shader(vertexShader8, fragmentShader8);
    // FileWatcher::add(vertexShader8, []()
    //                  { reloadShader = true; });
    // FileWatcher::add(fragmentShader8, []()
    //                  { reloadShader = true; });

    const char *vertexShader9 = "assets/shaders/ssao.vert";
    const char *fragmentShader9 = "assets/shaders/ssao_blur.frag";
    shaderSSAOBlur = new Shader(vertexShader9, fragmentShader9);
    FileWatcher::add(vertexShader9, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader9, []()
                     { reloadShader = true; });

    // Initialise lights
    light = new DirLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    plight = new PointLight(glm::vec3(-0.5f, -2.2f, -2.5f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.7, 1.8);
    coral = new Material(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    glm::vec3 viewPos = Window::getCamera()->getPosition();

    skinningShader->use();
    skinningShader->setDirLight("light", light);
    skinningShader->setPointLight("plight", plight);
    skinningShader->setMaterial("material", coral);
    skinningShader->setVec3("viewPos", viewPos);

    // shaderLightingPass->use();
    // shaderLightingPass->setDirLight("light", light);
    // shaderLightingPass->setPointLight("plight", plight);
    // shaderLightingPass->setMaterial("material", coral);
    // shaderLightingPass->setVec3("viewPos", viewPos);

    // Shadow mapping
    shadowMap = new ShadowMap();
    shadowMap->init();

    gbuffer = new GBuffer();

    glGenFramebuffers(1, &ssaoFBO);
    glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);

    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Window::getWidth(), Window::getHeight(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;

    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Window::getWidth(), Window::getHeight(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);
        ssaoNoise.push_back(noise);
    }
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    // shaderLightingPass->use();
    // shaderLightingPass->setInt("gPosition", 0);
    // shaderLightingPass->setInt("gNormal", 1);
    // shaderLightingPass->setInt("gAlbedo", 2);
    // shaderLightingPass->setInt("ssao", 3);
    
    // Blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Init start application time for animation
    StartTimeMillis = glfwGetTime() * 1000;
}

void Renderer::free()
{
    // Delete shaders
    delete skinningShader;
    delete depthSkinning;
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,
            1.0f,
            0.0f,
            0.0f,
            1.0f,
            -1.0f,
            -1.0f,
            0.0f,
            0.0f,
            0.0f,
            1.0f,
            1.0f,
            0.0f,
            1.0f,
            1.0f,
            1.0f,
            -1.0f,
            0.0f,
            1.0f,
            0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

void Renderer::render()
{
    glEnable(GL_DEPTH_TEST);
    if (reloadShader)
    {
        std::cout << "reload ourShader" << std::endl;

        skinningShader->reload();
        depthSkinning->reload();
        // shaderLightingPass->reload();
        shaderSSAO->reload();

        reloadShader = false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDisable(GL_BLEND);
    gbuffer->render();

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    shaderSSAO->use();
    Window::setMatrices(shaderSSAO);
    // Send kernel + rotation
    for (unsigned int i = 0; i < 64; ++i)
    {
        std::string res = "samples[" + std::to_string(i) + "]";
        shaderSSAO->setVec3(res.c_str(), ssaoKernel[i]);
    }

    shaderSSAO->use();
    shaderSSAO->setInt("gPosition", 0);
    shaderSSAO->setInt("gNormal", 1);
    shaderSSAO->setInt("texNoise", 2);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    shaderSSAOBlur->use();
    shaderSSAOBlur->setInt("ssaoInput", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    renderQuad();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_BLEND);

    // Render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;

    float near_plane = 1.0f, far_plane = 35.0f;
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    lightView = glm::lookAt(-10.0f * light->direction, glm::vec3(0.0f, -0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    depthSkinning->use();
    depthSkinning->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shadowMap->renderActivate();
    World::render(depthSkinning);
    shadowMap->renderDeactivate();

    // Reset viewport
    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene normally
    // Skinning Shader
    skinningShader->use();
    Window::setMatrices(skinningShader);
    skinningShader->setDirLight("light", light);
    skinningShader->setPointLight("plight", plight);
    skinningShader->setMaterial("material", coral);
    skinningShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    skinningShader->setInt("shadowMap", 2);
    skinningShader->setInt("ssao", 3);
    skinningShader->setInt("gPosition", 4);
    skinningShader->setInt("gNormal", 5);
    skinningShader->setInt("gAlbedo", 6);

    shadowMap->bindShadowMap();
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glActiveTexture(GL_TEXTURE4); 
    glBindTexture(GL_TEXTURE_2D, gbuffer->gPosition);
    glActiveTexture(GL_TEXTURE5); 
    glBindTexture(GL_TEXTURE_2D, gbuffer->gNormal);
    glActiveTexture(GL_TEXTURE6); 
    glBindTexture(GL_TEXTURE_2D, gbuffer->gAlbedo);
    World::render(skinningShader);

    water->render();
}
