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

#include <chrono>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

// #include "Model.h"
// #include "AnimatedModel.h"

#include "FileWatcher.h"
#include "Camera.h"
#include "Shader.h"

#include "Renderer.h"
#include "World.h"

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
    World::init();
    Renderer::init();
    auto lastFrame = std::chrono::system_clock::now();
    // Some computation here

    // render loop
    while (!Window::shouldClose())
    {
        auto thisFrame = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = thisFrame-lastFrame;
        lastFrame = thisFrame;

        float tslf = static_cast<float>(elapsed_seconds.count());

        World::update(tslf);

        // input
        Window::processInput();
        Renderer::render();

        // check and call events and swap the buffersprocessInput(window);
        Window::swapBuffers();
        //sleep(1/120.0);
    }

    World::free();
    Renderer::free();
    FileWatcher::stop();

    return 0;
}

