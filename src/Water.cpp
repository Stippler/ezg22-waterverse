#include "Water.h"

#include <GL/glew.h>
#include <GL/gl.h>

#include "FileWatcher.h"

Water::Water(unsigned int width, unsigned int height) : width(width), height(height)
{
    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
            work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);

    test = new ComputeShader("assets/shaders/water/test.comp");
    waterShader = new Shader("assets/shaders/water/water.vert",
                             "assets/shaders/water/water.frag");
    texture = new WaterTexture(width, height);

    FileWatcher::add("assets/shaders/water/test.comp", [&]()
                     { reloadCompute = true; });

    FileWatcher::add("assets/shaders/water/water.vert", [&]()
                     { reloadShader = true; });

    FileWatcher::add("assets/shaders/water/water.frag", [&]()
                     { reloadShader = true; });

    float vertices[] = {
        // positions        // texture coords
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,   // top right
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, // bottom left
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        0, 1, 3, // first triangle
        1, 2, 3  // second triangle
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // texture coord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

Water::~Water()
{
    delete texture;
}

void Water::render()
{
    if (reloadCompute)
    {
        test->reload();
        std::cout << "reload test computing shader" << std::endl;
        reloadCompute = false;
    }

    test->use();
    texture->bindImage(0, GL_WRITE_ONLY);
    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
    // glFinish();

    if (reloadShader)
    {
        waterShader->reload();
        std::cout << "reload water shader" << std::endl;
        reloadShader = false;
    }

    // bind textures

    waterShader->use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    waterShader->setInt("tex", 0);
    texture->bind(0);
    // texture->bindImage(0, GL_WRITE_ONLY);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}
