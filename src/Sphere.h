#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <GL/glew.h>
#include <GL/gl.h>
#include <vector>
#include <string>

#include "Shader.h"

class Sphere
{
public:

    Sphere(float radius=1.0f, int sectorCount=36, int stackCount=18, glm::vec3 position=glm::vec3(.0f));

    void draw(Shader *shader);

private:
    unsigned int VBO, VAO, EBO;
    unsigned int textureDiffuse, textureSpecular;

    std::string texturePath;
    glm::mat4 model = glm::mat4(1.0f);

	std::vector<float> interleavedVertices;
	std::vector<int> indices;
	std::vector<int> lineIndices;
};