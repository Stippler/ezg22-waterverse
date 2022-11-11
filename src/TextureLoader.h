#pragma once

#include <unordered_map>
#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include <string>
#include <GL/gl.h>
#include <iostream>

#include "stb_image.h"

namespace TextureLoader
{
    std::unordered_map<std::string, unsigned int> textures;

    unsigned int getTexture(std::string path)
    {
        unsigned int texture;
        auto entry = textures.find(path);
        if (entry == textures.end())
        {
            glGenTextures(1, &texture);
            // glBindTexture(GL_TEXTURE_2D, texture);
            //  set the texture wrapping/filtering options (on the currently bound texture object)
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // load and generate the texture
            int width, height, nrChannels;
            unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrChannels, 0);
            if (data)
            {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cerr << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        }
        else
        {
            texture = entry->second;
        }
        return texture;
    }
}