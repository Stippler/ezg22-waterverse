#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Shader.h"
#include "Vertex.h"

#include <string>
#include <vector>
using namespace std;

#define MAX_BONE_INFLUENCE 4

struct Texture {
    unsigned int id;
    string type;
    string path;
};

class Mesh {
public:
    // mesh Data
    vector<Vertex>       vertices;
    vector<unsigned int> indices;
    vector<Texture>      textures;
    unsigned int VAO;

    // constructor
    Mesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures);

    // render the mesh
    void draw(Shader &shader);

private:
    // render data 
    unsigned int VBO, EBO;

    // initializes all the buffer objects/arrays
    void setupMesh();
};