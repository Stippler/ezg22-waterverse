#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

#include "Shader.h"

class ShadowMap{
    public:
        ShadowMap(unsigned int textureDimension = 1024);
        void render();
        void bindShadowMap();
        
    private:
        unsigned int depthMapFBO;
        unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
        unsigned int depthMap;
        unsigned int depthMapSkinning;
        
        bool reloadShader=false;

        Shader *depthSkinning;
};