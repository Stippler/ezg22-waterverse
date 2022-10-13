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
#include "Water.h"

// Water
Water *water;


Shader *ourShader;
bool reloadShader = false;

unsigned int modelLoc, viewLoc, projectionLoc;

// predefines
void initOurShader();

std::vector<Model *> models;
std::vector<Cube *> cubes;

struct DirLight
{
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    DirLight(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
    {
        this->direction = direction;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
    }
};

struct PointLight
{
    glm::vec3 position;

    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;

    PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic)
    {
        this->position = position;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
};

struct Material
{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess)
    {
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
        this->shininess = shininess;
    }
};

void Renderer::init()
{
    water = new Water(512, 512);

    // Rest:

    const char *vertexShader = "assets/shaders/vertex.vert";
    const char *fragmentShader = "assets/shaders/fragment.frag";
    
    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });

    cubes.push_back(new Cube("assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    cubes.push_back(new Cube("assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    cubes.push_back(new Cube("assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));

    models.push_back(new Model("assets/models/fish/fish.obj"));
    models.push_back(new Model("assets/models/tigershark/untitled.obj", glm::vec3(5.0f, 5.0f, -15.0f)));
    models.push_back(new Model("assets/models/floor/floor.obj", glm::vec3(.0f, -5.0f, 0.f)));

    ourShader = new Shader(vertexShader, fragmentShader);
    ourShader->use();

    DirLight light(glm::vec3(1.0f, -1.0f, 1.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    ourShader->setVec3("light.ambient", light.ambient);
    ourShader->setVec3("light.diffuse", light.diffuse);
    ourShader->setVec3("light.specular", light.specular);
    ourShader->setVec3("light.direction", light.direction);

    PointLight plight(glm::vec3(-0.5f, -2.2f, -2.5f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.7, 1.8);
    ourShader->setVec3("plight.ambient", plight.ambient);
    ourShader->setVec3("plight.diffuse", plight.diffuse);
    ourShader->setVec3("plight.specular", plight.specular);
    ourShader->setVec3("plight.position", plight.position);
    ourShader->setFloat("plight.constant", plight.constant);
    ourShader->setFloat("plight.linear", plight.linear);
    ourShader->setFloat("plight.quadratic", plight.quadratic);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    ourShader->setVec3("viewPos", viewPos);

    Material coral(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    ourShader->setVec3("material.ambient", coral.ambient);
    ourShader->setVec3("material.diffuse", coral.diffuse);
    ourShader->setVec3("material.specular", coral.specular);
    ourShader->setFloat("material.shininess", coral.shininess);

    glEnable(GL_DEPTH_TEST);
}

void Renderer::free()
{
    delete ourShader;
    delete water;
    for (auto model : models)
    {
        delete model;
    }
    for (auto cube : cubes)
    {
        delete cube;
    }
}

void Renderer::render()
{
    if (reloadShader)
    {
        ourShader->reload();
        std::cout << "reload ourShader" << std::endl;
        reloadShader = false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader->use();
    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
    ourShader->setMat4("projection", proj);
    auto viewMatrix = Window::getCamera()->getViewMatrix();
    ourShader->setMat4("view", viewMatrix);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    ourShader->setVec3("viewPos", viewPos);

    for (auto model : models)
    {
        model->draw(*ourShader);
    }

    for (auto cube : cubes)
    {
        cube->draw(*ourShader);
    }

    water->render();
}

void initOurShader()
{
}
