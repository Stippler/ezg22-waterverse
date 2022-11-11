#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>


struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
	//bone indexes which will influence this vertex
	glm::ivec4 m_BoneIDs;
	//weights from each bone
	glm::vec4 m_Weights;
};
