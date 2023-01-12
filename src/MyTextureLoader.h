#pragma once

#include <unordered_map>
#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include <string>
#include <GL/gl.h>
#include <iostream>
#include <exception>

#include "stb_image.h"

namespace MyTextureLoader
{

    void init();

    unsigned int getTexture(std::string path);
    unsigned int getCubemap(float timer);
    unsigned int loadCubemap(std::vector<std::string> faces);

};