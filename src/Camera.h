#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    Camera() = default;
    // constructor with scalar values
    Camera(float posX, float posY, float posZ);
    // TODO: Camera(float posX, float posY, float posZ, float yaw, float pitch);

    void moveForward(float deltaTime);
    void moveBackward(float deltaTime);
    void moveLeft(float deltaTime);
    void moveRight(float deltaTime);
    void moveUp(float deltaTime);
    void moveDown(float deltaTime);

    glm::mat4 getViewMatrix() const;

    glm::vec3 getPosition() const;

    void processMouseMovement(float xoffset, float yoffset);

private:
    // camera Attributes
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    // const glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yaw{};
    float pitch{};

    float mouseSensitivity= 0.03f;

    float movementSpeed = 5.0f;
};