#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

class WaterTexture
{
public:
    WaterTexture(unsigned int width, unsigned int height);
    ~WaterTexture();

    void bind(const unsigned int unit = 0);
    void bindImage(GLint unit, GLenum access);
private:
    unsigned int width,  height;
    GLint format;
    GLuint textureId;
};