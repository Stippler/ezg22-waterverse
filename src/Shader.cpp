#include "Shader.h"
#include <exception>
#include <vector>

Shader::Shader(const char *vertexPath, const char *fragmentPath) : vertexPath(vertexPath), fragmentPath(fragmentPath)
{
    if (!this->reload())
    {
        throw std::exception();
    }
}

bool Shader::reload()
{
    // 1. retrieve the vertex/fragment source code from filePath
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // ensure ifstream objects can throw exceptions:
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try
    {
        // open files
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // read file's buffer contents into streams
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        // close file handlers
        vShaderFile.close();
        fShaderFile.close();
        // convert stream into string
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    }
    catch (std::ifstream::failure &e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ: " << e.what() << std::endl;
        return false;
    }
    const char *vShaderCode = vertexCode.c_str();
    const char *fShaderCode = fragmentCode.c_str();
    // 2. compile shaders
    unsigned int vertex, fragment;
    // vertex shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    if (!checkCompileErrors(vertex, "VERTEX"))
    {
        return false;
    }
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    if (!checkCompileErrors(fragment, "FRAGMENT"))
    {
        return false;
    }
    // shader Program
    unsigned int newId = glCreateProgram();
    glAttachShader(newId, vertex);
    glAttachShader(newId, fragment);
    glLinkProgram(newId);
    if (!checkCompileErrors(newId, "PROGRAM"))
    {
        return false;
    }
    ID = newId;
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return true;
}
// activate the shader
// ------------------------------------------------------------------------
void Shader::use()
{
    glUseProgram(ID);
}
// utility uniform functions
// ------------------------------------------------------------------------
void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
// ------------------------------------------------------------------------
void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
// ------------------------------------------------------------------------
void Shader::setFloat(const std::string &name, float value) const
{
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setMat4(const GLchar *name, glm::mat4 matrix)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(matrix));
}


void Shader::setVec2(const GLchar *name, glm::vec2 &vector)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniform2fv(projectionLoc, 1, &vector[0]);
}

void Shader::setVec3(const GLchar *name, glm::vec3 &vector)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniform3fv(projectionLoc, 1, &vector[0]);
}

void Shader::setFloat(const GLchar *name, float value)
{
    unsigned int projectionLoc = glGetUniformLocation(ID, name);
    glUniform1f(projectionLoc, value);
}

void Shader::setDirLight(const GLchar *name, DirLight *light){
    // TODO fix:
    std::string prefix = std::string(name);
    std::string suffix = std::string(".ambient");
    std::string res = prefix+suffix;
    this->setVec3(res.c_str(), light->ambient);
    suffix = std::string(".diffuse");
    res = prefix+suffix;
    this->setVec3(res.c_str(), light->diffuse);
    suffix = std::string(".specular");
    res = prefix+suffix;
    this->setVec3(res.c_str(), light->specular);
    suffix = std::string(".direction");
    res = prefix+suffix;
    this->setVec3(res.c_str(), light->direction);
}

void Shader::setPointLight(const GLchar *name, std::vector<PointLight *> plight){
    // TODO fix:
    for(int i=0; i<plight.size(); i++){
        std::string prefix = std::string(name)+std::string("[")+std::to_string(i)+std::string("]");
        std::string suffix = std::string(".ambient");
        std::string res = prefix+suffix;
        this->setVec3(res.c_str(), plight[i]->ambient);
        suffix = std::string(".diffuse");
        res = (prefix + suffix);
        this->setVec3(res.c_str(), plight[i]->diffuse);
        suffix = std::string(".specular");
        res = (prefix + suffix);
        this->setVec3(res.c_str(), plight[i]->specular);
        suffix = std::string(".position");
        res = (prefix + suffix);
        this->setVec3(res.c_str(), plight[i]->position);
        suffix = std::string(".constant");
        res = (prefix + suffix);
        this->setFloat(res.c_str(), plight[i]->constant);
        suffix = std::string(".linear");
        res = (prefix + suffix);
        this->setFloat(res.c_str(), plight[i]->linear);
        suffix = std::string(".quadratic");
        res = (prefix + suffix);
        this->setFloat(res.c_str(), plight[i]->quadratic);
    }
    
}

void Shader::setMaterial(const GLchar *name, Material *material){
    std::string prefix = std::string(name);
    std::string suffix = std::string(".ambient");
    std::string res = prefix + suffix;
    this->setVec3(res.c_str(), material->ambient);
    suffix = std::string(".diffuse");
    res = prefix + suffix;
    this->setVec3(res.c_str(), material->diffuse);
    suffix = std::string(".specular");
    res = prefix + suffix;
    this->setVec3(res.c_str(), material->specular);
    suffix = std::string(".shininess");
    res = prefix + suffix;
    this->setFloat(res, material->shininess);
}

// utility function for checking shader compilation/linking errors.
// ------------------------------------------------------------------------
bool Shader::checkCompileErrors(unsigned int shader, std::string type)
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
