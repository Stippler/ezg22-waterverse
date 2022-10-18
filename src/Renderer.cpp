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
#include "ShadowMap.h"
#include "DirLight.h"
#include "PointLight.h"
#include "Material.h"

#include <random>

// Water
Water *water;

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

Shader *shaderGeometryPass;
Shader *shaderSSAO;
Shader *shaderLightingPass;
Shader *shaderSSAOBlur;

Model *tigershark;
AnimatedModel *whiteshark;
AnimatedModel *fish;
Model *ground;
Model *pokeball;
Model *crate;

// Shadow predefines
ShadowMap *shadowMap;

//lights predefines
DirLight *light;
PointLight *plight;
Material *coral;

//animation predefines
long long StartTimeMillis = 0;

//ssao init
unsigned int gBuffer, gPosition, gNormal, gAlbedo;
unsigned int ssaoFBO, ssaoBlurFBO;
std::vector<glm::vec3> ssaoKernel;
unsigned int noiseTexture;
unsigned int ssaoColorBuffer, ssaoColorBufferBlur;


float ourLerp(float a, float b, float f)
{
    return a + f * (b - a);
}

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

    const char *vertexShader6 = "assets/shaders/ssao_geometry.vert";
    const char *fragmentShader6 = "assets/shaders/ssao_geometry.frag";
    shaderGeometryPass = new Shader(vertexShader6, fragmentShader6);
    FileWatcher::add(vertexShader6, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader6, []()
                     { reloadShader = true; });

    const char *vertexShader7 = "assets/shaders/ssao.vert";
    const char *fragmentShader7 = "assets/shaders/ssao.frag";
    shaderSSAO = new Shader(vertexShader7, fragmentShader7);
    FileWatcher::add(vertexShader7, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader7, []()
                     { reloadShader = true; });

    const char *vertexShader8 = "assets/shaders/ssao.vert";
    const char *fragmentShader8 = "assets/shaders/ssao_lighting.frag";
    shaderLightingPass = new Shader(vertexShader8, fragmentShader8);
    FileWatcher::add(vertexShader8, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader8, []()
                     { reloadShader = true; });

    const char *vertexShader9 = "assets/shaders/ssao.vert";
    const char *fragmentShader9 = "assets/shaders/ssao_blur.frag";
    shaderSSAOBlur = new Shader(vertexShader9, fragmentShader9);
    FileWatcher::add(vertexShader9, []()
                     { reloadShader = true; });
    FileWatcher::add(fragmentShader9, []()
                     { reloadShader = true; });

    
    //stbi_set_flip_vertically_on_load(true);

    //fish = new Model("assets/models/fish/fish.obj");
    //rock = new Model("assets/models/rock/Rock1/Rock1.obj");
    //tigershark = new Model("assets/models/tigershark/untitled.obj", glm::vec3(0.0, -8.0, -0.0));
    /*stbi_set_flip_vertically_on_load(true);
    crate = new Model("assets/models/backpack/backpack.obj");
    stbi_set_flip_vertically_on_load(false);*/
    whiteshark = new AnimatedModel("assets/models/whiteshark/WhiteShark.gltf", glm::vec3(0.0, -8.0, -0.0));
    fish = new AnimatedModel("assets/models/guppy-fish/Guppy.gltf", glm::vec3(0.0, -8.0, 12.0));
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
    shadowMap = new ShadowMap();
    shadowMap->init();

    // ssao
    glGenFramebuffers(1, &gBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

    glGenTextures(1, &gPosition);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Window::getWidth(), Window::getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

    glGenTextures(1, &gNormal);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, Window::getWidth(), Window::getHeight(), 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

    glGenTextures(1, &gAlbedo);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Window::getWidth(), Window::getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedo, 0);

    unsigned int attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
    glDrawBuffers(3, attachments);

    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, Window::getWidth(), Window::getHeight());
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &ssaoFBO);  glGenFramebuffers(1, &ssaoBlurFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    
    // SSAO color buffer
    glGenTextures(1, &ssaoColorBuffer);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Window::getWidth(), Window::getHeight(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Framebuffer not complete!" << std::endl;
    // and blur stage
    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glGenTextures(1, &ssaoColorBufferBlur);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, Window::getWidth(), Window::getHeight(), 0, GL_RED, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlur, 0);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0); 


    std::uniform_real_distribution<float> randomFloats(0.0, 1.0); // random floats between [0.0, 1.0]
    std::default_random_engine generator;
    for (unsigned int i = 0; i < 64; ++i)
    {
        glm::vec3 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator));
        sample = glm::normalize(sample);
        sample *= randomFloats(generator);
        float scale = float(i) / 64.0f;

        // scale samples s.t. they're more aligned to center of kernel
        scale = ourLerp(0.1f, 1.0f, scale * scale);
        sample *= scale;
        ssaoKernel.push_back(sample);
    }

    std::vector<glm::vec3> ssaoNoise;
    for (unsigned int i = 0; i < 16; i++)
    {
        glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); 
        ssaoNoise.push_back(noise);
    } 
    glGenTextures(1, &noiseTexture);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    shaderLightingPass->use();
    shaderLightingPass->setInt("gPosition", 0);
    shaderLightingPass->setInt("gNormal", 1);
    shaderLightingPass->setInt("gAlbedo", 2);
    shaderLightingPass->setInt("ssao", 3);
    shaderSSAO->use();
    shaderSSAO->setInt("gPosition", 0);
    shaderSSAO->setInt("gNormal", 1);
    shaderSSAO->setInt("texNoise", 2);
    shaderSSAOBlur->use();
    shaderSSAOBlur->setInt("ssaoInput", 0);

    // Blending
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); 

    // Init start application time for animation
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

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
    // initialize (if necessary)
    if (cubeVAO == 0)
    {
        float vertices[] = {
            // back face
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
             1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
            -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
            -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
            // front face
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
             1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
            -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
            -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
            // left face
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
            -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
            // right face
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
             1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
             1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
             1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
            // bottom face
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
             1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
             1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
            -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
            -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
            // top face
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
             1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
             1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
             1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
            -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
            -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
        };
        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        // fill buffer
        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        // link vertex attributes
        glBindVertexArray(cubeVAO);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }
    // render Cube
    glBindVertexArray(cubeVAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
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
    glBindVertexArray(0);
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
        shaderLightingPass->reload();
        shaderSSAO->reload();
        reloadShader = false;
    }

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Animation
    long long CurrentTimeMillis = glfwGetTime() * 1000;
    float AnimationTimeSec = ((float)(CurrentTimeMillis - StartTimeMillis)) / 1000.0f;

    vector<aiMatrix4x4> sharkTransforms;
    int MAX_BONES = 100;
    glm::mat4 sharkTransform[MAX_BONES] = {};
    whiteshark->getBoneTransforms(AnimationTimeSec, sharkTransforms);
    for (int i = 0; i < sharkTransforms.size(); i++) {
        glm::mat4 t = glm::mat4(1.0f);
        t[0][0] = sharkTransforms[i][0][0];
        t[0][1] = sharkTransforms[i][0][1];
        t[0][2] = sharkTransforms[i][0][2];
        t[0][3] = sharkTransforms[i][0][3];
        t[1][0] = sharkTransforms[i][1][0];
        t[1][1] = sharkTransforms[i][1][1];
        t[1][2] = sharkTransforms[i][1][2];
        t[1][3] = sharkTransforms[i][1][3];
        t[2][0] = sharkTransforms[i][2][0];
        t[2][1] = sharkTransforms[i][2][1];
        t[2][2] = sharkTransforms[i][2][2];
        t[2][3] = sharkTransforms[i][2][3];
        t[3][0] = sharkTransforms[i][3][0];
        t[3][1] = sharkTransforms[i][3][1];
        t[3][2] = sharkTransforms[i][3][2];
        t[3][3] = sharkTransforms[i][3][3];
        sharkTransform[i] = t;
    }

    vector<aiMatrix4x4> fishTransforms;
    glm::mat4 fishTransform[MAX_BONES] = {};
    fish->getBoneTransforms(AnimationTimeSec, fishTransforms);
    for (int i = 0; i < fishTransforms.size(); i++) {
        glm::mat4 t = glm::mat4(1.0f);
        t[0][0] = fishTransforms[i][0][0];
        t[0][1] = fishTransforms[i][0][1];
        t[0][2] = fishTransforms[i][0][2];
        t[0][3] = fishTransforms[i][0][3];
        t[1][0] = fishTransforms[i][1][0];
        t[1][1] = fishTransforms[i][1][1];
        t[1][2] = fishTransforms[i][1][2];
        t[1][3] = fishTransforms[i][1][3];
        t[2][0] = fishTransforms[i][2][0];
        t[2][1] = fishTransforms[i][2][1];
        t[2][2] = fishTransforms[i][2][2];
        t[2][3] = fishTransforms[i][2][3];
        t[3][0] = fishTransforms[i][3][0];
        t[3][1] = fishTransforms[i][3][1];
        t[3][2] = fishTransforms[i][3][2];
        t[3][3] = fishTransforms[i][3][3];
        fishTransform[i] = t;
    }

    // ssao
    glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
    glm::mat4 view = Window::getCamera()->getViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    shaderGeometryPass->use();
    shaderGeometryPass->setMat4("projection", projection);
    shaderGeometryPass->setMat4("view", view);
    shaderGeometryPass->setInt("animated", 1); 
    glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass->ID, "gBones"), sharkTransforms.size(), GL_TRUE, glm::value_ptr(sharkTransform[0]));
    whiteshark->draw(*shaderGeometryPass);
    glUniformMatrix4fv(glGetUniformLocation(shaderGeometryPass->ID, "gBones"), fishTransforms.size(), GL_TRUE, glm::value_ptr(fishTransform[0]));
    fish->draw(*shaderGeometryPass);
    shaderGeometryPass->setInt("animated", 0); 
    crate->draw(*shaderGeometryPass);
    ground->draw(*shaderGeometryPass);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    shaderSSAO->use();
    // Send kernel + rotation 
    for (unsigned int i = 0; i < 64; ++i){
        std::string res = "samples["+std::to_string(i)+"]";
        shaderSSAO->setVec3(res.c_str(), ssaoKernel[i]);
    }
    shaderSSAO->setMat4("projection", projection);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, noiseTexture);
    //renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurFBO);
    glClear(GL_COLOR_BUFFER_BIT);
    shaderSSAOBlur->use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
    //renderQuad();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // 4. lighting pass: traditional deferred Blinn-Phong lighting with added screen-space ambient occlusion
    // -----------------------------------------------------------------------------------------------------
    /*glm::vec3 lightPos = glm::vec3(2.0, 6.0, -2.0);
    glm::vec3 lightColor = glm::vec3(0.2, 0.2, 0.7);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    shaderLightingPass->use();
    // send light relevant uniforms
    glm::vec3 lightPosView = glm::vec3(Window::getCamera()->getViewMatrix() * glm::vec4(lightPos, 1.0));
    shaderLightingPass->setVec3("light.position", lightPosView);
    shaderLightingPass->setVec3("light.color", lightColor);
    // Update attenuation parameters
    const float linear    = 0.09f;
    const float quadratic = 0.032f;
    shaderLightingPass->setFloat("light.linear", linear);
    shaderLightingPass->setFloat("light.quadratic", quadratic);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gPosition);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, gNormal);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, gAlbedo);
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);
    renderQuad();*/
    
    // Render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;
    float near_plane = 1.0f, far_plane = 35.0f;
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    lightView = glm::lookAt(-10.0f*light->direction, glm::vec3(0.0f, -0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;

    depthSkinning->use();
    depthSkinning->setMat4("lightSpaceMatrix", lightSpaceMatrix);

    shadowMap->renderActivate();
    
    depthSkinning->setInt("animated", 1);
    glUniformMatrix4fv(glGetUniformLocation(depthSkinning->ID, "gBones"), sharkTransforms.size(), GL_TRUE, glm::value_ptr(sharkTransform[0]));
    whiteshark->draw(*depthSkinning);
    glUniformMatrix4fv(glGetUniformLocation(depthSkinning->ID, "gBones"), fishTransforms.size(), GL_TRUE, glm::value_ptr(fishTransform[0]));
    fish->draw(*depthSkinning);
    depthSkinning->setInt("animated", 0);
    //crate->draw(*depthSkinning);
    crate->draw(*depthSkinning);
    ground->draw(*depthSkinning);

    shadowMap->renderDeactivate();
    
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
    shadowShader->setInt("shadowMap", 2);
    skinningShader->use();
    skinningShader->setInt("shadowMap", 2);
    shadowMap->bindShadowMap();
    shadowShader->use();
    shadowShader->setInt("ssao", 3);
    skinningShader->use();
    skinningShader->setInt("ssao", 3);
    glActiveTexture(GL_TEXTURE3); // add extra SSAO texture to lighting pass
    glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlur);

    glUniformMatrix4fv(glGetUniformLocation(skinningShader->ID, "gBones"), sharkTransforms.size(), GL_TRUE, glm::value_ptr(sharkTransform[0]));
    whiteshark->draw(*skinningShader);
    glUniformMatrix4fv(glGetUniformLocation(skinningShader->ID, "gBones"), fishTransforms.size(), GL_TRUE, glm::value_ptr(fishTransform[0]));
    fish->draw(*skinningShader);
    shadowShader->use();
    crate->draw(*shadowShader);
    //crate->draw(*shadowShader);
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
