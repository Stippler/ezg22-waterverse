#pragma once

#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>

class ComputeShader
{
public:
    unsigned int ID;

    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    ComputeShader(const char *computePath);

    bool reload();

    // activate the shader
    // ------------------------------------------------------------------------
    void use();
    // utility uniform functions
    // ------------------------------------------------------------------------
    void setBool(const std::string &name, bool value) const;
    // ------------------------------------------------------------------------
    void setInt(const std::string &name, int value) const;
    // ------------------------------------------------------------------------
    void setFloat(const std::string &name, float value) const;

    void setMat4(const GLchar* name, glm::mat4 matrix);

    void setVec2(const GLchar* name, glm::vec2 &vector);

    void setVec3(const GLchar* name, glm::vec3 &vector);

    void setFloat(const GLchar *name, float value);

private:

    const char *computePath;
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    bool checkCompileErrors(unsigned int shader, std::string type);
};
