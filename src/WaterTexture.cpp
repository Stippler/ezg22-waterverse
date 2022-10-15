#include "WaterTexture.h"

WaterTexture::WaterTexture(unsigned int width, unsigned int height) : width(width), height(height)
{
    glGenTextures(1, &textureId);
    // glActiveTexture(GL_TEXTURE0);
    // glBindTexture(GL_TEXTURE_2D, textureId);
    bind();

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA,
                 GL_FLOAT, NULL);

    glBindTexture(GL_TEXTURE_2D, 0);
    // glBindImageTexture(0, textureId, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    format = GL_RGBA32F;
}

WaterTexture::~WaterTexture()
{}

void WaterTexture::bind(const unsigned int unit)
{
    glActiveTexture(GL_TEXTURE0+unit);
    glBindTexture(GL_TEXTURE_2D, textureId);
}

void WaterTexture::bindImage(GLint unit, GLenum access)
{
    glBindImageTexture(unit, textureId, 0, GL_FALSE, 0, access, format);
}