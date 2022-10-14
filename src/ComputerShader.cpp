#include "ComputeShader.h"

ComputeShader::ComputeShader(const char *computePath) : computePath(computePath)
{
    reload();
}

bool ComputeShader::reload()
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string computeCode;
    std::ifstream cShaderFile;
    // ensure ifstream objects can throw exceptions:
    cShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        cShaderFile.open(computePath);
        std::stringstream cShaderStream;
        // read file's buffer contents into streams
        cShaderStream << cShaderFile.rdbuf();
        // close file handlers
        cShaderFile.close();
        // convert stream into string
        computeCode = cShaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        return false;
    }
    const char *cShaderCode = computeCode.c_str();

    // unsigned int compute;
    // compute shader
    GLuint compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, &cShaderCode, NULL);
    glCompileShader(compute);
    if (!checkCompileErrors(compute, "COMPUTE"))
    {
        return false;
    }

    // shader Program
    GLuint newId = glCreateProgram();
    glAttachShader(newId, compute);
    glLinkProgram(newId);
    if (!checkCompileErrors(newId, "PROGRAM"))
    {
        return false;
    }
    ID = newId;
    glDeleteShader(compute);
}

// activate the shader
// ------------------------------------------------------------------------
void ComputeShader::use()
{
    glUseProgram(ID);
}
// utility uniform functions
// ------------------------------------------------------------------------
void ComputeShader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void ComputeShader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void ComputeShader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void ComputeShader::setMat4(const GLchar *name, glm::mat4 matrix)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}

void ComputeShader::setVec3(const GLchar *name, glm::vec3 &vector)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniform3fv(projectionLoc, 1, &vector[0]);
}

void ComputeShader::setVec2(const GLchar *name, glm::vec2 &vector)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniform2fv(projectionLoc, 1, &vector[0]);
}

void ComputeShader::setFloat(const GLchar *name, float value)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniform1f(projectionLoc, value);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
bool ComputeShader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
                      << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    return success;
}
