#pragma once

#include <GL/glew.h>
#include <GL/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

#include "Shader.h"

class CubeMap{
    public:
        CubeMap(glm::vec3 lightPos, unsigned int textureDimension = 1024);
        void render();
        unsigned int depthCubeMap;
        //void bindShadowMap();
        
    private:
        unsigned int depthMapFBO;
        unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
        
        
        bool reloadShader=false;

        glm::mat4 shadowProj;
        glm::vec3 lightPos;
        std::vector<glm::mat4> shadowTransforms;
        float far;

        Shader *shaderDepthCubeMap;
};