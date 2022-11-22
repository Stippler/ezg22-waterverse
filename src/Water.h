#pragma once

#include <glm/glm.hpp>

#include "ComputeShader.h"
#include "Shader.h"
#include "WaterTexture.h"

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

    void render();
    void renderCaustics(unsigned int environment);
    unsigned int causticsFBO, caustics;
private:

    unsigned int width, height;
    unsigned int VBO, VAO, EBO;

    unsigned int VBO2, VAO2, EBO2;

    unsigned int idx=0;

    glm::mat4 model;

    ComputeShader *normalCompute;
    ComputeShader *dropCompute;
    ComputeShader *updateCompute;

    WaterTexture *texture;
    WaterTexture *copyTexture;

    Shader *waterShader;
    Shader *causticsShader;

    bool reloadCompute = false;
    bool reloadShader = false;

    void swapTexture();
};