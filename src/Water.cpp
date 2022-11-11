#include "Water.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include <assert.h>

#include <vector>

#include "FileWatcher.h"
#include "Window.h"

#include "World.h"

Water::Water(unsigned int width, unsigned int height) : width(width), height(height)
{
    assert(width % 2 == 0);
    assert(height % 2 == 0);

    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
           work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    normalCompute = new ComputeShader("assets/shaders/water/normal.comp");
    updateCompute = new ComputeShader("assets/shaders/water/update.comp");
    dropCompute = new ComputeShader("assets/shaders/water/drop.comp");

    waterShader = new Shader("assets/shaders/water/water.vert",
                             "assets/shaders/water/water.frag");

    std::cout << width << "   " << height << std::endl;
    texture = new WaterTexture(width, height);
    copyTexture = new WaterTexture(width, height);

    FileWatcher::add("assets/shaders/water/normal.comp", [&]()
                     { reloadCompute = true; });
    FileWatcher::add("assets/shaders/water/update.comp", [&]()
                     { reloadCompute = true; });
    FileWatcher::add("assets/shaders/water/drop.comp", [&]()
                     { reloadCompute = true; });

    FileWatcher::add("assets/shaders/water/water.vert", [&]()
                     { reloadShader = true; });
    FileWatcher::add("assets/shaders/water/water.frag", [&]()
                     { reloadShader = true; });

    std::vector<float> vertices(height * width * 5);
    std::vector<unsigned int> indices(height * width * 2 * 3);

    idx = 0;
    for (int z = 0; z < height; z++)
    {
        for (int x = 0; x < width; x++)
        {
            float u = ((float)x / width);
            float v = ((float)z / height);

            vertices[idx++] = (float)x / width;
            vertices[idx++] = 0.0f;
            vertices[idx++] = (float)z / height;
            vertices[idx++] = u;
            vertices[idx++] = v;
        }
    }

    idx = 0;
    for (unsigned int z = 0; z < height - 1; z++)
    {
        for (unsigned int x = 0; x < width - 1; x++)
        {
            indices[idx++] = (width * z) + x;       // 0
            indices[idx++] = (width * (z + 1)) + x; // 2
            indices[idx++] = (width * z) + x + 1;   // 1

            indices[idx++] = (width * z) + x + 1;       // 1
            indices[idx++] = (width * (z + 1)) + x;     // 2
            indices[idx++] = (width * (z + 1)) + x + 1; // 3
        }
    }

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // vertex textureCoordinates
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

Water::~Water()
{
    delete texture;
}

void Water::update(float tslf)
{
    if (reloadCompute)
    {
        normalCompute->reload();
        dropCompute->reload();
        updateCompute->reload();

        std::cout << "reload test computing shader" << std::endl;
        reloadCompute = false;
    }
    static int ledl = 0;

    if(ledl%100==0) {
        addDrop(glm::vec2(0.5, 0), 0.1, 3);
        addDrop(glm::vec2(-0.5, 0), 0.1, 3);
    }
    ledl++;

    stepSimulation();
    stepSimulation();
    updateNormals();
}

void Water::render()
{
    if (reloadShader)
    {
        waterShader->reload();
        std::cout << "reload water shader" << std::endl;
        reloadShader = false;
    }

    // bind textures
    waterShader->use();
    texture->bind(0);

    Window::setMatrices(waterShader);
    waterShader->setDirLight("light", World::getDirLight());
    waterShader->setMat4("model", model);
    waterShader->setInt("tex", 0);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(GL_TRIANGLES, idx, GL_UNSIGNED_INT, 0);
}

void Water::addDrop(glm::vec2 center, float radius, float strength)
{
    dropCompute->use();
    dropCompute->setVec2("center", center);
    dropCompute->setFloat("radius", radius);
    dropCompute->setFloat("strength", strength);
    texture->bindImage(0, GL_READ_ONLY);
    copyTexture->bindImage(1, GL_WRITE_ONLY);
    glDispatchCompute(texture->width, texture->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    swapTexture();
}

void Water::stepSimulation()
{
    updateCompute->use();
    texture->bindImage(0, GL_READ_ONLY);
    copyTexture->bindImage(1, GL_WRITE_ONLY);
    glDispatchCompute(texture->width, texture->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    swapTexture();
}

void Water::updateNormals()
{
    normalCompute->use();
    texture->bindImage(0, GL_READ_ONLY);
    copyTexture->bindImage(1, GL_WRITE_ONLY);
    glDispatchCompute(texture->width, texture->height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    swapTexture();
}

void Water::swapTexture()
{
    WaterTexture *temp = copyTexture;
    copyTexture = texture;
    texture = temp;
}