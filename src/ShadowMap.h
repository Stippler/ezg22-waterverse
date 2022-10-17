#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

#include "Shader.h"

class ShadowMap{
    public:
        ShadowMap(unsigned int textureDimension = 1024);
        void init();
        void renderActivate();
        void renderDeactivate();
        void bindShadowMap();
        
    private:
        unsigned int depthMapFBO;
        unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
        unsigned int depthMap;
        unsigned int depthMapSkinning;
};