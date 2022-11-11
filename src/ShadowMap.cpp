#include "ShadowMap.h"
#include "FileWatcher.h"
#include "World.h"
#include "Window.h"

#include <glm/glm.hpp>

ShadowMap::ShadowMap(const unsigned int textureDimension)
{
    const char *vertexShader = "assets/shaders/depthSkinning.vert";
    const char *fragmentShader = "assets/shaders/depth.frag";
    depthSkinning = new Shader(vertexShader, fragmentShader);

    FileWatcher::add(vertexShader, [&]()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, [&]()
                     { reloadShader = true; });

    this->SHADOW_HEIGHT = textureDimension;
    this->SHADOW_WIDTH = textureDimension;

    glEnable(GL_DEPTH_TEST);
    glGenFramebuffers(1, &this->depthMapFBO);

    glGenTextures(1, &this->depthMap);
    glBindTexture(GL_TEXTURE_2D, this->depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = {1.0, 1.0, 1.0, 1.0};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, this->depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void ShadowMap::render()
{
    if (reloadShader)
    {
        std::cout << "Reload ShadowMap Shader" << std::endl;
        depthSkinning->reload();
        reloadShader = false;
    }

    depthSkinning->use();
    depthSkinning->setMat4("lightSpaceMatrix", World::getLightSpaceMatrix());

    glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, this->depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_CULL_FACE);
    World::renderGameObjects(depthSkinning);
    glCullFace(GL_BACK);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Reset viewport
    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void ShadowMap::bindShadowMap()
{
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, this->depthMap);
}