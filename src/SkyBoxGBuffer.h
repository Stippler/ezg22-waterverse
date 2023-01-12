#pragma once

#include "MyTextureLoader.h"
#include "Shader.h"
#include "Window.h"
#include "World.h"

struct SkyBoxGBuffer
{
    Shader *shader;
    unsigned int VAO, VBO;

    unsigned int gAlbedo;
    unsigned int gBuffer;

    SkyBoxGBuffer()
    {
        MyTextureLoader::init();
        shader = new Shader(
            "assets/shaders/sky/skybox.vert",
            "assets/shaders/sky/skybox.frag");
        float vertices[] = {
            // positions
            -1.0f, 1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,

            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f};

        // skybox VAO
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);

        shader->use();
        shader->setInt("cubeMap", 0);

        resize();
    }

    void resize()
    {
        static bool initialized = false;
        if (initialized)
        {
            glDeleteFramebuffers(1, &gBuffer);
            glDeleteTextures(1, &gBuffer);
            // glDeleteTextures(1, &gNormal);
            // glDeleteTextures(1, &gAlbedo);
            // glDeleteRenderbuffers(1, &rboDepth);
            initialized = true;
        }
        float width = Window::getWidth();
        float height = Window::getHeight();
        // ssao
        glGenFramebuffers(1, &gBuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);

        glGenTextures(1, &gAlbedo);
        glBindTexture(GL_TEXTURE_2D, gAlbedo);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gAlbedo, 0);

        unsigned int attachments[1] = {GL_COLOR_ATTACHMENT0};
        glDrawBuffers(1, attachments);

        // finally check if framebuffer is complete
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            std::cout << "Framebuffer not complete!" << std::endl;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    ~SkyBoxGBuffer()
    {
        delete shader;
    }

    void render()
    {
        glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // set opengl properties
        glDepthFunc(GL_LEQUAL);
        // glFrontFace(GL_CW);
        glEnable(GL_CULL_FACE);


        unsigned int cubemapTexture = MyTextureLoader::getCubemap(World::getTime());

        // set shader stuff
        shader->use();
        glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)Window::getWidth() / (float)Window::getHeight(), 0.1f, 100.0f);
        Window::setMatrices(shader);
        shader->setMat4("projection", proj);
        glm::mat4 view = glm::mat4(glm::mat3(Window::getCamera()->getViewMatrix()));
        shader->setMat4("view", view);
        unsigned int skyboxTexture = MyTextureLoader::getCubemap(World::getTime());

        // render
        glBindVertexArray(VAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        // free stuff
        glBindVertexArray(0);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_CULL_FACE);
        glDepthFunc(GL_LESS);
        // glFrontFace(GL_CCW);
    }
};