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
#include "World.h"

// Water
Water *water;
GBuffer *gbuffer;
SSAO *ssao;

bool reloadShader = false;

unsigned int modelLoc, viewLoc, projectionLoc;

// init depth shader
Shader *skinningShader;

// Shadow predefines
ShadowMap *shadowMap;

// lights predefines
Material *coral;

// animation predefines
long long StartTimeMillis = 0;


void Renderer::init()
{
    glEnable(GL_DEPTH_TEST);

    water = new Water();

    const char *vertexShader5 = "assets/shaders/skinning.vert";
    const char *fragmentShader5 = "assets/shaders/skinning.frag";
    skinningShader = new Shader(vertexShader5, fragmentShader5);
    FileWatcher::add(vertexShader5, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader5, []()
                     { reloadShader = true; });

    
    // Initialise lights
    
    coral = new Material(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    glm::vec3 viewPos = Window::getCamera()->getPosition();

    skinningShader->use();
    skinningShader->setDirLight("light", World::getDirLight());
    skinningShader->setPointLight("plight", World::getPointLight());
    skinningShader->setMaterial("material", coral);
    skinningShader->setVec3("viewPos", viewPos);

    shadowMap = new ShadowMap();
    gbuffer = new GBuffer();
    ssao = new SSAO(gbuffer);
    
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
}

void Renderer::render()
{
    glEnable(GL_DEPTH_TEST);
    if (reloadShader)
    {
        std::cout << "reload ourShader" << std::endl;

        skinningShader->reload();
        // shaderLightingPass->reload();
        // shaderSSAO->reload();

        reloadShader = false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);
    gbuffer->render();
    ssao->render();
    glEnable(GL_BLEND);

    
    shadowMap->render();

    // Reset viewport
    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene normally
    // Skinning Shader
    skinningShader->use();
    Window::setMatrices(skinningShader);
    skinningShader->setDirLight("light", World::getDirLight());
    skinningShader->setPointLight("plight", World::getPointLight());
    skinningShader->setMaterial("material", coral);
    skinningShader->setMat4("lightSpaceMatrix", World::getLightSpaceMatrix());
    skinningShader->setInt("shadowMap", 2);
    skinningShader->setInt("ssao", 3);
    skinningShader->setInt("gPosition", 4);
    skinningShader->setInt("gNormal", 5);
    skinningShader->setInt("gAlbedo", 6);

    // ssaoColorBufferBlur
    shadowMap->bindShadowMap();
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, ssao->ssaoColorBufferBlur);
    glActiveTexture(GL_TEXTURE4); 
    glBindTexture(GL_TEXTURE_2D, gbuffer->gPosition);
    glActiveTexture(GL_TEXTURE5); 
    glBindTexture(GL_TEXTURE_2D, gbuffer->gNormal);
    glActiveTexture(GL_TEXTURE6); 
    glBindTexture(GL_TEXTURE_2D, gbuffer->gAlbedo);
    World::render(skinningShader);

    water->render();
}
