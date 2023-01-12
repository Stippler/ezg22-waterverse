#pragma once

#include <glm/glm.hpp>

#include "ComputeShader.h"
#include "Shader.h"
#include "WaterTexture.h"
#include "MyTextureLoader.h"

struct Drop
{
    float start;
    glm::vec2 center;
    float radius;
    float strength;
};

struct GridVertex
{
    glm::vec3 pos;
    glm::vec2 texCoords;
};

class Water
{
public:
    Water(unsigned int width = 256, unsigned int height = 256);
    ~Water();

    void addDrop(glm::vec2 center, float radius, float strength);
    void stepSimulation();
    void updateNormals();

    void update(float tslf);

    void render(Shader *waterShader);
    void renderCaustics(unsigned int environment);
    unsigned int causticsFBO, caustics;
    float totalTimer=0.0f;

private:
    unsigned int width, height;
    unsigned int VBO, VAO, EBO;

    unsigned int VBO2, VAO2, EBO2;

    unsigned int cubeVAO, cubeVBO;

    unsigned int idx = 0;

    glm::mat4 modelWater = glm::scale(glm::mat4(1.0f), glm::vec3(30 - 0.966666, 30 - 0.966666, 30 - 0.966666));
    glm::mat4 modelGlass = glm::scale(glm::mat4(1.0f), glm::vec3(30, 30, 30));

    glm::mat4 modelCaustics = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(30 - 2 * 0.966666, 1, 30 - 2 * 0.966666)), glm::vec3(1, 1, 1));

    ComputeShader *normalCompute;
    ComputeShader *dropCompute;
    ComputeShader *updateCompute;

    WaterTexture *texture;
    WaterTexture *copyTexture;

    Shader *causticsShader;
    Shader *environmentShader;

    bool reloadCompute = false;
    bool reloadShader = false;
    bool reloadDrops = false;

    float tick = 1 / 60.0f;
    float timer = 0.0f;
    float totalTime = 0.0f;

    std::vector<Drop> drops;
    int dropIndex = 0;
    float dropTimer=0.0f;
    float totalDropTime;

    void swapTexture();
    void reload();
};