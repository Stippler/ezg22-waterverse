#include "GameObject.h"

#include "Shader.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/rotate_vector.hpp>

#define MAX_BONES 100

GameObject::GameObject(AnimatedModel *model, bool animated) : model(model), animated(animated)
{
}

GameObject::~GameObject()
{
}

void GameObject::render(Shader *shader)
{

    glm::mat4 modelMatrix(1.0f);
    modelMatrix = glm::translate(modelMatrix, pos);
    modelMatrix = glm::scale(modelMatrix, glm::vec3(scale));
    float vel_length = glm::length(velocity);

    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    if (abs(vel_length) > 0.00000001)
    {
        modelMatrix *= glm::orientation(glm::normalize(velocity), model->axis);
    }
    else
    {
        modelMatrix *= glm::orientation(glm::normalize(glm::vec3(1, 0, 0)), model->axis);
    }

    shader->setMat4("model", modelMatrix);

    if (animated)
    {
        vector<aiMatrix4x4> transforms;
        glm::mat4 transform[MAX_BONES] = {};
        model->getBoneTransforms(time, transforms);
        for (int i = 0; i < transforms.size(); i++)
        {
            glm::mat4 t = glm::mat4(1.0f);
            t[0][0] = transforms[i][0][0];
            t[0][1] = transforms[i][0][1];
            t[0][2] = transforms[i][0][2];
            t[0][3] = transforms[i][0][3];
            t[1][0] = transforms[i][1][0];
            t[1][1] = transforms[i][1][1];
            t[1][2] = transforms[i][1][2];
            t[1][3] = transforms[i][1][3];
            t[2][0] = transforms[i][2][0];
            t[2][1] = transforms[i][2][1];
            t[2][2] = transforms[i][2][2];
            t[2][3] = transforms[i][2][3];
            t[3][0] = transforms[i][3][0];
            t[3][1] = transforms[i][3][1];
            t[3][2] = transforms[i][3][2];
            t[3][3] = transforms[i][3][3];
            transform[i] = t;
        }
        // TODO: change this to shader->set sth
        glUniformMatrix4fv(glGetUniformLocation(shader->ID, "gBones"), transforms.size(), GL_TRUE, glm::value_ptr(transform[0]));
    }
    model->draw(*shader);
}