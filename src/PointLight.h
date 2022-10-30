#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct PointLight {
    glm::vec3 position;  
  
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
	
    float constant;
    float linear;
    float quadratic;

    PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);
    glm::vec3 getPos();
};