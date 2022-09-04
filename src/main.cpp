#define STB_IMAGE_IMPLEMENTATION

#include "Window.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <sstream>
#include <unordered_map>

#include "FileWatcher.h"
#include "Model.h"

#include "Camera.h"
#include "Shader.h"

#include "Renderer.h"

unsigned int create_texture();
void loadShader();

int main(const int argc, const char **argv)
{
    Window::init();

    glewExperimental = true;
    GLenum err = glewInit();

    if (err != GLEW_OK)
    {
        std::cerr << "Could not initialize GLEW" << std::endl;
        return -2;
    }

    
    unsigned int textureId = create_texture();

    stbi_set_flip_vertically_on_load(true);

    FileWatcher::start();
    Renderer::init();

    glEnable(GL_DEPTH_TEST);

    // render loop
    while (!Window::shouldClose())
    {
        // input
        Window::processInput();

        Renderer::render();

        // check and call events and swap the buffersprocessInput(window);
        Window::swapBuffers();
    }

    Renderer::free();
    FileWatcher::stop();

    return 0;
}

unsigned int create_texture()
{
    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load and generate the texture
    int width, height, nrChannels;
    unsigned char *data = stbi_load("../assets/container.jpg", &width, &height, &nrChannels, 0);
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
    return texture;
}