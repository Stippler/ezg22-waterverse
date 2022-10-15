#pragma once

#include <GL/glew.h>
#include <GL/gl.h>

class WaterTexture
{
public:
    WaterTexture(unsigned int width=512, unsigned int height=512);
    ~WaterTexture();

    void bind(const unsigned int unit = 0);
    void bindImage(GLint unit, GLenum access);

    unsigned int width,  height;
private:
    GLint format;
    GLuint textureId;
};