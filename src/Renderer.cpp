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

// init depth shader
Shader *depthShader;
Shader *shadowShader;
Shader *debugShadow;

Model *shark;
Model *fish;
Model *ground;
Model *pokeball;
Model *crate;
std::vector<Cube> cubes;

// Shadow predefines
unsigned int depthMapFBO;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
unsigned int depthMap;

// Window predefines
//int width = 800;
//int height = 600;

// Structs for lights
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

//lights predefines
DirLight *light;

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
    ourShader = new Shader(vertexShader, fragmentShader);

    FileWatcher::add(vertexShader, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader, []()
                     { reloadShader = true; });

    vertexShader = "assets/shaders/depth.vert";
    fragmentShader = "assets/shaders/depth.frag";
    depthShader = new Shader(vertexShader, fragmentShader);

    vertexShader = "assets/shaders/shadow.vert";
    fragmentShader = "assets/shaders/shadow.frag";
    shadowShader = new Shader(vertexShader, fragmentShader);

    vertexShader = "assets/shaders/debugShadow.vert";
    fragmentShader = "assets/shaders/debugShadow.frag";
    debugShadow = new Shader(vertexShader, fragmentShader);

    initOurShader();

    //cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(0.0f, 0.0f, 0.0f)));
    cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(2.0f, 5.0f, -15.0f)));
    cubes.push_back(*new Cube("assets/container.jpg", glm::vec3(-1.5f, -2.2f, -2.5f)));

    //stbi_set_flip_vertically_on_load(true);

    //ourModel = new Model("assets/models/beach_umbrella/12984_beach_umbrella_v1_L2.obj");
    //stbi_set_flip_vertically_on_load(true);
    //ourModel = new Model("assets/models/backpack/backpack.obj");
    fish = new Model("assets/models/fish/fish.obj");
    //rock = new Model("assets/models/rock/Rock1/Rock1.obj");
    shark = new Model("assets/models/tigershark/untitled.obj");
    ground = new Model("assets/models/floor/floor.obj");
    crate = new Model("assets/models/Crate/Crate1.obj");
    //pokeball = new Model("C:/Users/chris/Downloads/pokeball/Pokeball.obj");


    // Initialise lights
    light = new DirLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    ourShader->setVec3("light.ambient", light->ambient);
    ourShader->setVec3("light.diffuse", light->diffuse);
    ourShader->setVec3("light.specular", light->specular);
    ourShader->setVec3("light.direction", light->direction);
    shadowShader->setVec3("light.ambient", light->ambient);
    shadowShader->setVec3("light.diffuse", light->diffuse);
    shadowShader->setVec3("light.specular", light->specular);
    shadowShader->setVec3("light.direction", light->direction);

    PointLight plight(glm::vec3(-0.5f, -2.2f, -2.5f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.7, 1.8);
    ourShader->setVec3("plight.ambient", plight.ambient);
    ourShader->setVec3("plight.diffuse", plight.diffuse);
    ourShader->setVec3("plight.specular", plight.specular);
    ourShader->setVec3("plight.position", plight.position);
    ourShader->setFloat("plight.constant", plight.constant);
    ourShader->setFloat("plight.linear", plight.linear);
    ourShader->setFloat("plight.quadratic", plight.quadratic);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    shadowShader->setVec3("viewPos", viewPos);
    shadowShader->setVec3("plight.ambient", plight.ambient);
    shadowShader->setVec3("plight.diffuse", plight.diffuse);
    shadowShader->setVec3("plight.specular", plight.specular);
    shadowShader->setVec3("plight.position", plight.position);
    shadowShader->setFloat("plight.constant", plight.constant);
    shadowShader->setFloat("plight.linear", plight.linear);
    shadowShader->setFloat("plight.quadratic", plight.quadratic);
    shadowShader->setVec3("viewPos", viewPos);

    Material coral(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.5f, 0.5f, 0.5f), 32.0f);
    ourShader->setVec3("material.ambient", coral.ambient);
    ourShader->setVec3("material.diffuse", coral.diffuse);
    ourShader->setVec3("material.specular", coral.specular);
    ourShader->setFloat("material.shininess", coral.shininess);
    shadowShader->setVec3("material.ambient", coral.ambient);
    shadowShader->setVec3("material.diffuse", coral.diffuse);
    shadowShader->setVec3("material.specular", coral.specular);
    shadowShader->setFloat("material.shininess", coral.shininess);

    // Shadow mapping
    // create a frame buffer object for rendering the depth map
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

void Renderer::render()
{
    if (reloadShader)
    {
        ourShader->reload();
        initOurShader();
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 35.0f;
    lightProjection = glm::ortho(-25.0f, 25.0f, -25.0f, 25.0f, near_plane, far_plane);
    //lightView = myLookAt(light->direction, glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    //lightView = myLookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.f), glm::vec3(0.f, 1.f, 0.f));
    lightView = glm::lookAt(-10.0f*light->direction, glm::vec3(0.0f, -10.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    
	lightSpaceMatrix = lightProjection * lightView;

    depthShader->use();
    depthShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_CULL_FACE);
    glm::mat4 mod = glm::mat4(1.0f);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -8.0, -0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    shark->draw(*depthShader);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-3.5, -5.2, -2.5))*glm::scale(mod,glm::vec3(.2,.2,.2)));
    fish->draw(*depthShader);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -5.0f, 7.0f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*depthShader);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, -2.5f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*depthShader);
    depthShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    ground->draw(*depthShader);
    glCullFace(GL_BACK);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    

    

    glViewport(0, 0, Window::getWidth(), Window::getHeight());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    /*ourShader->use();
    auto viewMatrix = Window::getCamera()->getViewMatrix();
    ourShader->setMat4("view", viewMatrix);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    ourShader->setVec3("viewPos", viewPos);

    mod = glm::mat4(1.0f);
    ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -8.0, -0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    shark->draw(*ourShader);
    ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-3.5, -5.2, -2.5))*glm::scale(mod,glm::vec3(.2,.2,.2)));
    fish->draw(*ourShader);
    ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -5.0f, 7.0f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*ourShader);
    ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, -2.5f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*ourShader);
    ourShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    ground->draw(*ourShader);*/


    shadowShader->use();
    auto viewMatrix = Window::getCamera()->getViewMatrix();
    shadowShader->setMat4("view", viewMatrix);
    glm::vec3 viewPos = Window::getCamera()->getPosition();
    shadowShader->setVec3("viewPos", viewPos);
    shadowShader->setMat4("lightSpaceMatrix", lightSpaceMatrix);
    glUniform1i(glGetUniformLocation(shadowShader->ID, "shadowMap"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, depthMap);

    mod = glm::mat4(1.0f);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -8.0, -0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    shark->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-3.5, -5.2, -2.5))*glm::scale(mod,glm::vec3(.2,.2,.2)));
    fish->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(2.0f, -5.0f, 7.0f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(-1.5f, -2.2f, -2.5f))*glm::scale(mod,glm::vec3(.5,.5,.5)));
    crate->draw(*shadowShader);
    shadowShader->setMat4("model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0, -10.0, 0.0))*glm::scale(mod,glm::vec3(1,1,1)));
    ground->draw(*shadowShader);


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

void Renderer::free()
{
    delete ourShader;
    delete shark;
    delete fish;
    delete ground;
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

    shadowShader->use();
    modelLoc = glGetUniformLocation(shadowShader->ID, "model");
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    viewLoc = glGetUniformLocation(shadowShader->ID, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    projectionLoc = glGetUniformLocation(shadowShader->ID, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(proj));
}
