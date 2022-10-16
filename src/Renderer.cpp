#include "Renderer.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <vector>

#include "Window.h"
#include "Shader.h"
#include "FileWatcher.h"
#include "ModelLoader.h"
#include "Cube.h"
#include "Water.h"
#include "AnimatedModel.h"

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
Shader *depthSkinning;
Shader *depthShader;
Shader *shadowShader;
Shader *debugShadow;
Shader *skinningShader;

Model *tigershark;
AnimatedModel *whiteshark;
Model *fish;
Model *ground;
Model *pokeball;
Model *crate;

// Shadow predefines
unsigned int depthMapFBO;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMap;
unsigned int depthMapSkinning;

//lights predefines
DirLight *light;
PointLight *plight;
Material *coral;

//animation predefines
long long StartTimeMillis = 0;

void Renderer::init()
{
    water = new Water();

    const char *vertexShader = "assets/shaders/depthSkinning.vert";
    const char *fragmentShader = "assets/shaders/depth.frag";
    depthSkinning = new Shader(vertexShader, fragmentShader);

    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });

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

    const char *vertexShader5 = "assets/shaders/skinning.vert";
    const char *fragmentShader5 = "assets/shaders/skinning.frag";
    skinningShader = new Shader(vertexShader5, fragmentShader5);
    FileWatcher::add(vertexShader5, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader5, []()
                     { reloadShader = true; });

    
    //stbi_set_flip_vertically_on_load(true);

    fish = new Model("assets/models/fish/fish.obj");
    //rock = new Model("assets/models/rock/Rock1/Rock1.obj");
    //tigershark = new Model("assets/models/tigershark/untitled.obj", glm::vec3(0.0, -8.0, -0.0));
    whiteshark = new AnimatedModel("assets/models/whiteshark/WhiteShark.gltf", glm::vec3(0.0, -8.0, -0.0));
    ground = new Model("assets/models/floor/floor.obj", glm::vec3(0.0, -10.0, 0.0));
    crate = new Model("assets/models/Crate/Crate1.obj");

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

    skinningShader->use();
    skinningShader->setDirLight("light", light);
    skinningShader->setPointLight("plight", plight);
    skinningShader->setMaterial("material", coral);
    skinningShader->setVec3("viewPos", viewPos);

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
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    StartTimeMillis = glfwGetTime()*1000; 
}

void Renderer::free()
{
    // Delete shaders
    delete shadowShader;
    delete skinningShader;
    delete depthShader;
    delete depthSkinning;
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
    delete tigershark;
    delete whiteshark;
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
        skinningShader->reload();
        depthShader->reload();
        depthSkinning->reload();
        reloadShader = false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Shark Animation
    long long CurrentTimeMillis = glfwGetTime() * 1000;
    float AnimationTimeSec = ((float)(CurrentTimeMillis - StartTimeMillis)) / 1000.0f;

    vector<aiMatrix4x4> Transforms;
    whiteshark->getBoneTransforms(AnimationTimeSec, Transforms);
    glm::mat4 Transform[100] = {};
    int MAX_BONES = 100;
    for (int i = 0; i < Transforms.size(); i++) {
        glm::mat4 t = glm::mat4(1.0f);
        t[0][0] = Transforms[i][0][0];
        t[0][1] = Transforms[i][0][1];
        t[0][2] = Transforms[i][0][2];
        t[0][3] = Transforms[i][0][3];
        t[1][0] = Transforms[i][1][0];
        t[1][1] = Transforms[i][1][1];
        t[1][2] = Transforms[i][1][2];
        t[1][3] = Transforms[i][1][3];
        t[2][0] = Transforms[i][2][0];
        t[2][1] = Transforms[i][2][1];
        t[2][2] = Transforms[i][2][2];
        t[2][3] = Transforms[i][2][3];
        t[3][0] = Transforms[i][3][0];
        t[3][1] = Transforms[i][3][1];
        t[3][2] = Transforms[i][3][2];
        t[3][3] = Transforms[i][3][3];
        Transform[i] = t;
    }
    skinningShader->use();
    glUniformMatrix4fv(glGetUniformLocation(skinningShader->ID, "gBones"), Transforms.size(), GL_TRUE, glm::value_ptr(Transform[0]));

    // Render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 35.0f;
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    lightView = glm::lookAt(-10.0f*light->direction, glm::vec3(0.0f, -0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;

    depthSkinning->use();
    Window::setMatrices(depthSkinning);
    depthSkinning->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_CULL_FACE);
    glm::mat4 mod = glm::mat4(1.0f);
    glUniformMatrix4fv(glGetUniformLocation(depthSkinning->ID, "gBones"), Transforms.size(), GL_TRUE, glm::value_ptr(Transform[0]));
    glUniform1i(glGetUniformLocation(depthSkinning->ID, "animated"), 1);
    whiteshark->draw(*depthSkinning);
    glUniform1i(glGetUniformLocation(depthSkinning->ID, "animated"), 0);
    fish->draw(*depthSkinning);
    crate->draw(*depthSkinning);
    crate->draw(*depthSkinning);
    ground->draw(*depthSkinning);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Reset viewport
    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render scene normally
    skinningShader->use();
    skinningShader->setDirLight("light", light);
    skinningShader->setPointLight("plight", plight);
    skinningShader->setMaterial("material", coral);
    shadowShader->use();
    shadowShader->setDirLight("light", light);
    shadowShader->setPointLight("plight", plight);
    shadowShader->setMaterial("material", coral);

    Window::setMatrices(shadowShader);
    skinningShader->use();
    Window::setMatrices(skinningShader);
    shadowShader->use();

    shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    skinningShader->use();
    skinningShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    shadowShader->use();
    glUniform1i(glGetUniformLocation(shadowShader->ID, "shadowMap"), 2);
    skinningShader->use();
    glUniform1i(glGetUniformLocation(skinningShader->ID, "shadowMap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMap);


    whiteshark->draw(*skinningShader);
    shadowShader->use();
    fish->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -5.0f, 7.0f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, -2.5f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*shadowShader);
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
