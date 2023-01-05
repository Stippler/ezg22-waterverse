#pragma once

#include <glm/glm.hpp>

#include "ComputeShader.h"
#include "Shader.h"
#include "WaterTexture.h"
#include "MyTextureLoader.h"

struct GridVertex{
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
private:

    unsigned int width, height;
    unsigned int VBO, VAO, EBO;

    unsigned int VBO2, VAO2, EBO2;

    unsigned int cubeVAO, cubeVBO;

    unsigned int idx=0;

    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(30, 30, 30));

    glm::mat4 modelCaustics = glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(30, 30, 30)), glm::vec3(1,1,1));

    ComputeShader *normalCompute;
    ComputeShader *dropCompute;
    ComputeShader *updateCompute;

    WaterTexture *texture;
    WaterTexture *copyTexture;

    Shader *causticsShader;
    Shader *environmentShader;

    bool reloadCompute = false;
    bool reloadShader = false;

    void swapTexture();
};