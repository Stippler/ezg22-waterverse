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

#include <filesystem>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#include "FileWatcher.h"
#include "Model.h"

#include "Camera.h"
#include "Shader.h"

#include "Renderer.h"

void loadShader();

int main(const int argc, const char **argv)
{
    std::cout << "Current working directory: " << std::filesystem::current_path() << endl;

    Window::init();

    glewExperimental = true;
    GLenum err = glewInit();

    if (err != GLEW_OK)
    {
        std::cerr << "Could not initialize GLEW" << std::endl;
        return -2;
    }
    
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
        //sleep(1/120.0);
    }

    Renderer::free();
    FileWatcher::stop();

    return 0;
}

