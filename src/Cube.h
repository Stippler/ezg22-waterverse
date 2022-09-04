#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include <string>
#include <GL/gl.h>
#include"stb_image.h"
#include "Shader.h"
#include <iostream>

class Cube
{
public:
    glm::mat4 model = glm::mat4(1.0f);
    unsigned int VBO, VAO;
    unsigned int texture;
    std::string texturePath;

    Cube(std::string texturePath, glm::vec3 position);

    void init();

    void draw(Shader &shader);
};