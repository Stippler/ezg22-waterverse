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
#include "SSAO.h"
#include "CubeMap.h"
#include "World.h"
#include "WaterGBuffer.h"
#include "Quad.h"

// Water
GBuffer *gbuffer;
WaterGBuffer *waterGBuffer;
SSAO *ssao;

Quad *quad;

bool reloadShader = false;
bool resizeViewport = false;

unsigned int modelLoc, viewLoc, projectionLoc;

// init depth shader
Shader *skinningShader;
Shader *test;
Shader *mainShader;

// Shadow predefines
// ShadowMap *shadowMap;
CubeMap *cubeMap;

// lights predefines
Material *coral;

// animation predefines
long long StartTimeMillis = 0;

void Renderer::init()
{
    glEnable(GL_DEPTH_TEST);
    // water = new Water();

    const char *vertexShader5 = "assets/shaders/skinning.vert";
    const char *fragmentShader5 = "assets/shaders/skinning.frag";

    skinningShader = new Shader(vertexShader5, fragmentShader5);
    FileWatcher::add(vertexShader5, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader5, []()
                     { reloadShader = true; });


    const char *vertexMainShader = "assets/shaders/main.vert";
    const char *fragmentMainShader = "assets/shaders/main.frag";

    mainShader = new Shader(vertexMainShader, fragmentMainShader);
    FileWatcher::add(vertexMainShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentMainShader, []()
                     { reloadShader = true; });

    std::cout << "hello" << std::endl;

    // Initialise lights

    coral = new Material(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    glm::vec3 viewPos = Window::getCamera()->getPosition();

    skinningShader->use();
    skinningShader->setDirLight("light", World::getDirLight());
    skinningShader->setPointLight("plight", World::getPointLight());
    skinningShader->setMaterial("material", coral);
    skinningShader->setVec3("viewPos", viewPos);

    // shadowMap = new ShadowMap();
    cubeMap = new CubeMap(World::getPointLight()[0]->position);
    gbuffer = new GBuffer();
    waterGBuffer = new WaterGBuffer();
    ssao = new SSAO(gbuffer);
    quad = new Quad();

    // Blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Init start application time for animation
    StartTimeMillis = glfwGetTime() * 1000;
}

void Renderer::resize()
{
    resizeViewport = true;
}

void Renderer::free()
{
    // Delete shaders
    delete skinningShader;
}

void Renderer::render()
{
    glEnable(GL_DEPTH_TEST);
    if (reloadShader)
    {
        std::cout << "reload skinning/main shaders" << std::endl;
        skinningShader->reload();
        mainShader->reload();
        // shaderLightingPass->reload();
        // shaderSSAO->reload();

        reloadShader = false;
    }
    if (resizeViewport)
    {
        gbuffer->resize();
        waterGBuffer->resize();
        ssao->resize();
        resizeViewport = false;
    }

    // glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render Model GBuffer
    glDisable(GL_BLEND);
    gbuffer->render();
    gbuffer->renderEnvironment();
    ssao->render();

    waterGBuffer->render();
    glEnable(GL_BLEND);

    // Render Water GBuffer
    // TODO
    // shadowMap->render();
    cubeMap->render();
    World::renderCaustics(gbuffer->environment);

    // Render scene normally
    // Skinning Shader
    skinningShader->use();
    Window::setMatrices(skinningShader);
    Window::setScreenSize(skinningShader);
    skinningShader->setDirLight("light", World::getDirLight());
    skinningShader->setPointLight("plight", World::getPointLight());
    skinningShader->setMaterial("material", coral);
    skinningShader->setMat4("lightSpaceMatrix", World::getLightSpaceMatrix());
    skinningShader->setInt("shadowMap", 2);
    skinningShader->setInt("ssao", 3);
    skinningShader->setInt("gPosition", 4);
    skinningShader->setInt("gNormal", 5);
    skinningShader->setInt("gAlbedo", 6);
    skinningShader->setInt("cubeShadowMap", 7);
    skinningShader->setInt("caustics", 8);

    // set cubemap int
    // ssaoColorBufferBlur
    // shadowMap->bindShadowMap();
    // glActiveTexture(GL_TEXTURE2); // add extra SSAO texture to lighting pass
    // glBindTexture(GL_TEXTURE_2D, gbuffer->environment);
    // glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    // glBindTexture(GL_TEXTURE_2D, ssao->ssaoColorBufferBlur);
    // glActiveTexture(GL_TEXTURE4);
    // glBindTexture(GL_TEXTURE_2D, gbuffer->gPosition);
    // glActiveTexture(GL_TEXTURE5);
    // glBindTexture(GL_TEXTURE_2D, gbuffer->gNormal);
    // glActiveTexture(GL_TEXTURE6);
    // glBindTexture(GL_TEXTURE_2D, gbuffer->gAlbedo);
    // glActiveTexture(GL_TEXTURE7);
    // glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->depthCubeMap);
    // glActiveTexture(GL_TEXTURE8);
    // glBindTexture(GL_TEXTURE_2D, World::getCaustics());

    // bind cube map
    // World::renderGameObjects(skinningShader);
    mainShader->use();
    mainShader->setInt("gPosition", 0);
    mainShader->setInt("gNormal", 1);
    mainShader->setInt("gAlbedo", 2);
    mainShader->setInt("gEnvironment", 3);

    mainShader->setInt("gWaterPosition", 4);
    mainShader->setInt("gWaterNormal", 5);
    mainShader->setInt("gWaterAlbedo", 6);

    mainShader->setInt("cubeShadowMap", 7);
    mainShader->setInt("caustics", 8);
    mainShader->setInt("ssao", 9);
    mainShader->setInt("shadowMap", 10);

    mainShader->setDirLight("light", World::getDirLight());
    mainShader->setMaterial("material", coral);
    mainShader->setMat4("lightSpaceMatrix", World::getLightSpaceMatrix());
    mainShader->setPointLight("plight", World::getPointLight());

    Window::setScreenSize(mainShader);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    mainShader->setVec3("viewPos", viewPos);
    Window::setMatrices(mainShader);
    // mainShader->setMat4("model", model);
    // mainShader->setInt("tex", 0);


    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gbuffer->gAlbedo);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, gbuffer->environment);

    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, waterGBuffer->gPosition);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, waterGBuffer->gNormal);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, waterGBuffer->gAlbedo);
    glActiveTexture(GL_TEXTURE7);

    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap->depthCubeMap);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, World::getCaustics());
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_2D, ssao->ssaoColorBuffer);
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, gbuffer->environment);

    quad->render(mainShader);
    // World::renderWater();
}
