#include "Camera.h"
#include "FileWatcher.h"

Camera::Camera(float posX, float posY, float posZ)
{
    cameraPos.x = posX;
    cameraPos.y = posY;
    cameraPos.z = posZ;
    yaw = -0.0f;
    pitch = -0.0f;

    movement = new CameraMovement();
    movement->reload();
    FileWatcher::add("assets/camera.txt", [&]
                     { reloadMovement = true; });
    updateAngle();
}

Camera::~Camera()
{
    delete movement;
}

glm::mat4 Camera::getViewMatrix() const
{
    if (automatic)
    {
        return glm::lookAt(cameraPosAuto, cameraPosAuto + cameraFrontAuto, cameraUpAuto);
    }
    else
    {
        return glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    }
}

glm::vec3 Camera::getPosition() const
{
    if (automatic)
    {
        return cameraPosAuto;
    }
    else
    {
        return cameraPos;
    }
}

void Camera::moveForward(float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    cameraPos += glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z)) * velocity;
}

void Camera::moveBackward(float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    cameraPos -= glm::normalize(glm::vec3(cameraFront.x, 0, cameraFront.z)) * velocity;
}

void Camera::moveLeft(float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    glm::vec3 vec = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraPos -= vec * velocity;
}

void Camera::moveRight(float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    cameraPos += glm::normalize(glm::cross(cameraFront, worldUp)) * velocity;
}

void Camera::moveUp(float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    cameraPos += worldUp * velocity;
}

void Camera::moveDown(float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    cameraPos -= worldUp * velocity;
}

void Camera::updateAngle()
{
    // calculate the new Front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(newFront);
    // also re-calculate the Right and Up vector
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraFront, worldUp));
    cameraUp = glm::normalize(glm::cross(cameraRight, cameraFront));

    glm::vec3 newFrontAuto;
    newFrontAuto.x = cos(glm::radians(yawAuto)) * cos(glm::radians(pitchAuto));
    newFrontAuto.y = sin(glm::radians(pitchAuto));
    newFrontAuto.z = sin(glm::radians(yawAuto)) * cos(glm::radians(pitchAuto));
    cameraFrontAuto = glm::normalize(newFrontAuto);
    // also re-calculate the Right and Up vector
    // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    glm::vec3 cameraRightAuto = glm::normalize(glm::cross(cameraFrontAuto, worldUp));
    cameraUpAuto = glm::normalize(glm::cross(cameraRightAuto, cameraFrontAuto));
}

void Camera::processMouseMovement(float xoffset, float yoffset)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
    {
        pitch = 89.0f;
    }
    if (pitch < -89.0f)
    {
        pitch = -89.0f;
    }

    updateAngle();
}

void Camera::setAuto(bool mode)
{
    automatic = mode;
}

void Camera::update(float tslf)
{
    if (reloadMovement)
    {
        movement->reload();
    }
    time += tslf;
    CameraPos pos = movement->getPos(time);
    yawAuto = pos.yaw;
    pitchAuto = pos.pitch;
    cameraPosAuto = pos.pos;
    updateAngle();
}