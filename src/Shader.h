#pragma once

#include <string>
#include <fstream>
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <GL/gl.h>
#include <iostream>
#include <sstream>
#include <glm/gtc/type_ptr.hpp>
#include "Watchable.h"

// Add Filewatcher and reload shader on change
class Shader: Watchable
{
public:
    // constructor generates the shader on the fly
    // ------------------------------------------------------------------------
    Shader(const char *vertexPath, const char *fragmentPath);

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

private:
    unsigned int id;

    const char *vertexPath;
    const char *fragmentPath;
    // utility function for checking shader compilation/linking errors.
    // ------------------------------------------------------------------------
    bool checkCompileErrors(unsigned int shader, std::string type);
};