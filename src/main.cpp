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

unsigned int createTexture();
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

    
    //unsigned int textureId = createTexture();

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

