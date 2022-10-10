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

// Rest:
glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::mat4(1.0f);
glm::mat4 proj = glm::mat4(1.0f);

Shader *ourShader;
bool reloadShader = false;

unsigned int modelLoc, viewLoc, projectionLoc;

// predefines
void initOurShader();

// Model *shark;
// Model *fish;
// Model *ground;
// Model *pokeball;
std::vector<Model*> models;
std::vector<Cube> cubes;

struct DirLight{
    glm::vec3 direction;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    DirLight(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular){
        this->direction = direction;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
    }
};

struct PointLight {
    glm::vec3 position;  
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
	
    float constant;
    float linear;
    float quadratic;

    PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic){
        this->position = position;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
        this->constant = constant;
        this->linear = linear;
        this->quadratic = quadratic;
    }
};

struct Material{
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    Material(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float shininess){
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
        this->shininess = shininess;
    }
};

void Renderer::init()
{
    water = new Water(512, 512);

    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    proj = glm::perspective(glm::radians(45.0f), (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);

    const char *vertexShader = "assets/shaders/vertex.vert";
    const char *fragmentShader = "assets/shaders/fragment.frag";
    ourShader = new Shader(vertexShader, fragmentShader);

    initOurShader();

    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });

    //cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    // cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    // cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));   

    //stbi_set_flip_vertically_on_load(true);

    //ourModel = new Model("assets/models/beach_umbrella/12984_beach_umbrella_v1_L2.obj");
    //stbi_set_flip_vertically_on_load(true);
    //ourModel = new Model("assets/models/backpack/backpack.obj");
    //rock = new Model("assets/models/rock/Rock1/Rock1.obj");
    // Model *fish = new Model("assets/models/fish/fish.obj");
    // Model *shark = new Model("assets/models/tigershark/untitled.obj");
    // Model *ground = new Model("assets/models/floor/floor.obj");
    // models.push_back(fish);
    // models.push_back(shark);
    // models.push_back(ground);
    //pokeball = new Model("C:/Users/chris/Downloads/pokeball/Pokeball.obj");

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
    // TODO: delete models
    // delete shark;
    // delete fish;
    // delete ground;
}

void Renderer::render()
{
    if (reloadShader)
    {
        ourShader->reload();
        initOurShader();
        std::cout << "reload ourShader" << std::endl;
        reloadShader=false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    ourShader->use();
    auto viewMatrix = Window::getCamera()->getViewMatrix();
    ourShader->setMat4("view", viewMatrix);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    ourShader->setVec3("viewPos", viewPos);

    // for (unsigned int i = 0; i < cubes.size(); i++)
    // {
    //     cubes[i].draw(*ourShader);
    // }

    // glm::mat4 mod = glm::mat4(1.0f);
    // ourShader->setMat4("model", glm::scale(mod,glm::vec3(.2,.2,.2)));
    // ourModel->draw(*ourShader);
    // for (auto cube : cubes)
    // {
    //     cube.draw(*ourShader);
    // }

    glm::mat4 mod = glm::mat4(1.0f);
    //ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));

    // TODO: draw
    // ourShader->setMat4("model", glm::scale(mod, glm::vec3(.2,.2,.2)));
    // shark->draw(*ourShader);
    // ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-0.5, -2.2, -2.5))*glm::scale(mod,glm::vec3(.2,.2,.2)));
    // fish->draw(*ourShader);
    // ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    // ground->draw(*ourShader);

    for (auto cube : cubes)
    {
        cube.draw(*ourShader);
    }

    
    water->render();
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
