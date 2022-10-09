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

struct Light{
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    Light(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular){
        this->position = position;
        this->ambient = ambient;
        this->diffuse = diffuse;
        this->specular = specular;
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

void initGBuffer()
{
    unsigned int gAlbedoSpec;

    unsigned int gBuffer;
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    unsigned int gPosition, gNormal, gColorSpec;

    // - position color buffer
    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Window::getWidth(), Window::getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    // - normal color buffer
    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Window::getWidth(), Window::getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    // - color + specular color buffer
    glGenTextures(1, &gAlbedoSpec);
    glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Window::getWidth(), Window::getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

    // - tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, attachments);
}

void Renderer::init()
{
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
    proj = glm::perspective(glm::radians(45.0f), (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);

    const char *vertexShader = "assets/shaders/vertex.vert";
    const char *fragmentShader = "assets/shaders/fragment.frag";
    std::cout << "after load shader" << std::endl;
    ourShader = new Shader(vertexShader, fragmentShader);
    std::cout << "load shader" << std::endl;

    initOurShader();

    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });

    // cubes.push_back(*new Cube("../assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));   

    stbi_set_flip_vertically_on_load(true);

    ourModel = new Model("assets/models/backpack/backpack.obj");

    Light light(glm::vec3(1.2f, 1.0f, 2.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    ourShader->setVec3("light.ambient", light.ambient);
    ourShader->setVec3("light.diffuse", light.diffuse);
    ourShader->setVec3("light.specular", light.specular);
    ourShader->setVec3("light.position", light.position);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    ourShader->setVec3("viewPos", viewPos);

    Material coral(glm::vec3(1.0f, 0.5f, 0.31f), glm::vec3(1.0f, 0.5f, 0.31f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    ourShader->setVec3("material.ambient", coral.ambient);
    ourShader->setVec3("material.diffuse", coral.diffuse);
    ourShader->setVec3("material.specular", coral.specular);
    ourShader->setFloat("material.shininess", coral.shininess);

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
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    ourShader->setVec3("viewPos", viewPos);


    glm::mat4 mod = glm::mat4(1.0f);
    ourShader->setMat4("model", glm::scale(mod,glm::vec3(.2,.2,.2)));
    ourModel->draw(*ourShader);

    for (auto cube : cubes)
    {
        cube.draw(*ourShader);
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
