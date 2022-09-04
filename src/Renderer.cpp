#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>

#include "Window.h"
#include "Shader.h"
#include "FileWatcher.h"
#include "ModelLoader.h"
#include "Cube.h"

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 proj = glm::mat4(1.0f);

Shader *ourShader;
bool reloadShader = false;

unsigned int modelLoc, viewLoc, projectionLoc;

// predefines
void initOurShader();

Model *ourModel;
std::vector<Cube> cubes;

void Renderer::init()
{
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    proj = glm::perspective(glm::radians(45.0f), (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);

    const char *vertexShader = "../assets/shaders/vertex.vert";
    const char *fragmentShader = "../assets/shaders/fragment.frag";
    ourShader = new Shader(vertexShader, fragmentShader);

    initOurShader();

    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });

    cubes.push_back(*new Cube("../assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    cubes.push_back(*new Cube("../assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    cubes.push_back(*new Cube("../assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));   

    stbi_set_flip_vertically_on_load(true);


    glEnable(GL_DEPTH_TEST);
}

void Renderer::render()
{
    if (reloadShader)
    {
        ourShader->reload();
        initOurShader();
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader->use();
    auto viewMatrix = Window::getCamera()->getViewMatrix();
    ourShader->setMat4("view", viewMatrix);

    for (unsigned int i = 0; i < cubes.size(); i++)
    {
        cubes[i].draw(*ourShader);
    }

    // ourModel->draw(*ourShader);
    for(auto cube:cubes){
    //cube.draw(*ourShader);
    }
}

void Renderer::free()
{
    delete ourShader;
    delete ourModel;
}

void initOurShader()
{
    ourShader->use();

    modelLoc = glGetUniformLocation(ourShader->ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    viewLoc = glGetUniformLocation(ourShader->ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    projectionLoc = glGetUniformLocation(ourShader->ID, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));
}
