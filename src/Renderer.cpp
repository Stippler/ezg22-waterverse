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

#include "DirLight.h"
#include "PointLight.h"
#include "Material.h"

bool reloadShader = false;

unsigned int modelLoc, viewLoc, projectionLoc;

std::vector<Model *> models;
std::vector<Cube *> cubes;

// init depth shader
Shader *depthShader;
Shader *shadowShader;
Shader *debugShadow;

Model *shark;
Model *fish;
Model *ground;
Model *pokeball;
Model *crate;

// Shadow predefines
unsigned int depthMapFBO;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMap;

//lights predefines
DirLight *light;
PointLight *plight;
Material *coral;

void Renderer::init()
{
    water = new Water();
    const char *vertexShader2 = "assets/shaders/depth.vert";
    const char *fragmentShader2 = "assets/shaders/depth.frag";
    depthShader = new Shader(vertexShader2, fragmentShader2);

    FileWatcher::add(vertexShader2, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader2, []()
                     { reloadShader = true; });

    const char *vertexShader3 = "assets/shaders/shadow.vert";
    const char *fragmentShader3 = "assets/shaders/shadow.frag";
    shadowShader = new Shader(vertexShader3, fragmentShader3);

    FileWatcher::add(vertexShader3, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader3, []()
                     { reloadShader = true; });

    const char *vertexShader4 = "assets/shaders/debugShadow.vert";
    const char *fragmentShader4 = "assets/shaders/debugShadow.frag";
    debugShadow = new Shader(vertexShader4, fragmentShader4);

    FileWatcher::add(vertexShader4, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader4, []()
                     { reloadShader = true; });

    //cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    //cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    //cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));
    // cubes.push_back(new Cube("assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    // cubes.push_back(new Cube("assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    // cubes.push_back(new Cube("assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));

    // models.push_back(new Model("assets/models/fish/fish.obj"));
    // models.push_back(new Model("assets/models/tigershark/untitled.obj", glm::vec3(5.0f, 5.0f, -15.0f)));
    // models.push_back(new Model("assets/models/floor/floor.obj", glm::vec3(.0f, -5.0f, 0.f)));

    //ourModel = new Model("assets/models/beach_umbrella/12984_beach_umbrella_v1_L2.obj");
    //stbi_set_flip_vertically_on_load(true);
    //ourModel = new Model("assets/models/backpack/backpack.obj");
    fish = new Model("assets/models/fish/fish.obj");
    //rock = new Model("assets/models/rock/Rock1/Rock1.obj");
    shark = new Model("assets/models/tigershark/untitled.obj", glm::vec3(0.0, -8.0, -0.0));
    ground = new Model("assets/models/floor/floor.obj", glm::vec3(0.0, -10.0, 0.0));
    crate = new Model("assets/models/Crate/Crate1.obj");

    //pokeball = new Model("C:/Users/chris/Downloads/pokeball/Pokeball.obj");

    // Initialise lights
    light = new DirLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    plight = new PointLight(glm::vec3(-0.5f, -2.2f, -2.5f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.7, 1.8);
    coral = new Material(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    glm::vec3 viewPos = Window::getCamera()->getPosition();

    shadowShader->use();
    shadowShader->setDirLight("light", light);
    shadowShader->setPointLight("plight", plight);
    shadowShader->setMaterial("material", coral);
    shadowShader->setVec3("viewPos", viewPos);

    // Shadow mapping
    glEnable(GL_DEPTH_TEST);
    glGenFramebuffers(1, &depthMapFBO);

    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  

    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glEnable(GL_DEPTH_TEST);
}

void Renderer::free()
{
    // Delete shaders
    delete shadowShader;
    delete depthShader;
    delete debugShadow;
    // delete water;
    for (auto model : models)
    {
        delete model;
    }
    for (auto cube : cubes)
    {
        delete cube;
    }

    // Delete models
    delete shark;
    delete fish;
    delete ground;
    delete crate;
}

void Renderer::render()
{
    if (reloadShader)
    {
        std::cout << "reload ourShader" << std::endl;
        debugShadow->reload();
        shadowShader->reload();
        depthShader->reload();
        reloadShader = false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    // glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 35.0f;
    lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
    lightView = glm::lookAt(-10.0f*light->direction, glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;

    depthShader->use();
    Window::setMatrices(depthShader);
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_CULL_FACE);
    glm::mat4 mod = glm::mat4(1.0f);
    // depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -8.0, -0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    shark->draw(*depthShader);
    // depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-3.5, -5.2, -2.5))*glm::scale(mod,glm::vec3(.2,.2,.2)));
    fish->draw(*depthShader);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -5.0f, 7.0f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*depthShader);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, -2.5f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*depthShader);
    // depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    ground->draw(*depthShader);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Reset viewport
    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene normally
    shadowShader->use();
    shadowShader->setDirLight("light", light);
    shadowShader->setPointLight("plight", plight);
    shadowShader->setMaterial("material", coral);

    Window::setMatrices(shadowShader);
    // auto viewMatrix = Window::getCamera()->getViewMatrix();
    // shadowShader->setMat4("view", viewMatrix);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    shadowShader->setVec3("viewPos", viewPos);
    shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    glUniform1i(glGetUniformLocation(shadowShader->ID, "shadowMap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMap);

    mod = glm::mat4(1.0f);
    // shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -8.0, -0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    shark->draw(*shadowShader);
    // shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-3.5, -5.2, -2.5))*glm::scale(mod,glm::vec3(.2,.2,.2)));
    fish->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -5.0f, 7.0f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, -2.5f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*shadowShader);
    // shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    ground->draw(*shadowShader);

    water->render();

    /*debugShadow->use();
    debugShadow->setInt("depthMap", 0);
    debugShadow->setFloat("near_plane", near_plane);
    debugShadow->setFloat("far_plane", far_plane);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    unsigned int quadVAO = 0;
    unsigned int quadVBO;
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
             1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);*/
    
}

