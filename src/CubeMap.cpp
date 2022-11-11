#include "CubeMap.h"
#include "Window.h"
#include "FileWatcher.h"
#include "World.h"

CubeMap::CubeMap(glm::vec3 lightPos, unsigned int textureDimension){
    this->lightPos = lightPos;
    const char *vertexShader = "assets/shaders/cubeShadow.vert";
    const char *fragmentShader = "assets/shaders/cubeShadow.frag";
    const char *geometryShader = "assets/shaders/cubeShadow.geom";
    shaderDepthCubeMap = new Shader(vertexShader, fragmentShader, geometryShader);

    FileWatcher::add(vertexShader, [&]()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, [&]()
                     { reloadShader = true; });
    FileWatcher::add(geometryShader, [&]()
                     { reloadShader = true; });

    SHADOW_WIDTH = 1024; 
    SHADOW_HEIGHT = 1024;
    glGenFramebuffers(1, &depthMapFBO);
    glGenTextures(1, &depthCubeMap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depthCubeMap);
    for (unsigned int i = 0; i < 6; ++i){
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);  

    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthCubeMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 

    float aspect = (float)SHADOW_WIDTH/(float)SHADOW_HEIGHT;
    float near = 1.0f;
    float far = 25.0f;
    this->far = far;
    shadowProj = glm::perspective(glm::radians(90.0f), aspect, near, far);

    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3(-1.0, 0.0, 0.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 1.0, 0.0), glm::vec3(0.0, 0.0, 1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0,-1.0, 0.0), glm::vec3(0.0, 0.0,-1.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0, 1.0), glm::vec3(0.0,-1.0, 0.0)));
    shadowTransforms.push_back(shadowProj * glm::lookAt(lightPos, lightPos + glm::vec3( 0.0, 0.0,-1.0), glm::vec3(0.0,-1.0, 0.0)));
}

void CubeMap::render(){
    if (reloadShader)
    {
        std::cout << "Reload ShadowMap Shader" << std::endl;
        shaderDepthCubeMap->reload();
        reloadShader = false;
    }

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    shaderDepthCubeMap->use();
    for (unsigned int i = 0; i < 6; ++i)
                shaderDepthCubeMap->setMat4(("shadowMatrices[" + std::to_string(i) + "]").c_str(), shadowTransforms[i]);

    shaderDepthCubeMap->setFloat("far_plane", this->far);
    shaderDepthCubeMap->setVec3("lightPos", lightPos);
    World::renderGameObjects(shaderDepthCubeMap);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}