#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <glm/glm.hpp>
#include "FileWatcher.h"
#include <iostream>
#include <fstream>
#include <string>

struct CameraPos
{
    glm::vec3 pos;
    float yaw;
    float pitch;
};

struct Curve
{
    std::vector<CameraPos> cameraPos;

    Curve(std::vector<CameraPos> cameraPos) : cameraPos(cameraPos)
    {
    }

    CameraPos interpolate(CameraPos pos1, CameraPos pos2, float t)
    {
        CameraPos inter;
        inter.pos = pos1.pos * t + pos2.pos * (1 - t);
        inter.yaw = pos1.yaw * t + pos2.yaw * (1 - t);
        inter.pitch = pos1.pitch * t + pos2.pitch * (1 - t);
        return inter;
    }

    CameraPos casteljau(float t, std::vector<CameraPos> points)
    {
        if (points.size() == 1)
        {
            return points[0];
        }
        std::vector<CameraPos> new_points;
        for (int i = 0; i < points.size() - 1; i++)
        {
            CameraPos inter = interpolate(points[i], points[i + 1], t);
            new_points.push_back(inter);
        }
        return casteljau(t, new_points);
    }

    CameraPos getCameraPos(float t)
    {
        return casteljau(t, cameraPos);
    }


    // Curve(std::vector<glm::vec3> points) : points(points)
    // {
    // }

    // glm::vec3 casteljau(float t, std::vector<glm::vec3> points)
    // {
    //     if (points.size() == 1)
    //     {
    //         return points[0];
    //     }
    //     std::vector<glm::vec3> new_points;
    //     for (int i = 0; i < points.size() - 1; i++)
    //     {
    //         new_points.push_back(points[i] * t + points[i + 1] * (1 - t));
    //     }
    //     return casteljau(t, new_points);
    // }

    // glm::vec3 getPos(float t)
    // {
    //     return casteljau(t, points);
    // }

};

struct CameraMovement
{

    std::vector<Curve> curves;
    // std::vector<float> yaws;
    // std::vector<float> pitches;
    std::vector<float> times;
    float totalTime=0.0f;

    CameraMovement()
    {
    }

    void reload()
    {
        std::vector<Curve> curves;
        std::vector<float> times;

        std::ifstream file;
        file.open("assets/camera.txt");

        int curveCount;
        file >> curveCount;
        totalTime=0.0f;
        times.clear();
        curves.clear();
        for (int i = 0; i < curveCount; i++)
        {
            int controlPoints;
            float time;
            file >> controlPoints;
            file >> time;
            totalTime+=time;

            std::vector<CameraPos> cameraPositions;
            for (int j = 0; j < controlPoints; j++)
            {
                CameraPos camPos;
                glm::vec3 pos;
                file >> pos.x >> pos.y >> pos.z;
                float yaw, pitch;
                file >> yaw >> pitch;
                camPos.pos = pos;
                camPos.yaw = yaw;
                camPos.pitch = pitch;
                cameraPositions.push_back(camPos);
            }
            Curve curve(cameraPositions);
            curves.push_back(curve);
            times.push_back(time);
        }
        file.close();
        this->curves.assign(curves.begin(), curves.end());
        this->times.assign(times.begin(), times.end());
    }

    CameraPos getPos(float time)
    {
        assert(times.size() == curves.size());
        assert(times.size() > 0);

        
        time = std::fmod(time, totalTime);

        int idx = -1;
        float cur_time = 0;
        for (int i = 0; i < times.size(); i++)
        {
            if (cur_time + times[i] > time)
            {
                idx = i;
                break;
            }
            else
            {
                cur_time += times[i];
            }
        }

        // times: [2, 3, 1]
        // time: 5.3
        // idx: 2 (to select 1)
        // t: (5.3-5)/1

        float t = (time - cur_time) / times[idx];
        CameraPos cameraPos = curves[idx].getCameraPos(t);
        // cameraPos.pos = ;
        // cameraPos.yaw = yaw[idx] * t + yaw[idx + 1] * (1 - t);
        // cameraPos.pitch = pitch[idx] * t + pitch[idx + 1] * (1 - t);
        return cameraPos;
    }
};

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
    Camera() = default;
    ~Camera();
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

    void setAuto(bool mode);

    float yaw{};
    float pitch{};

    void update(float tslf);

private:
    void updateAngle();

    // camera Attributes
    glm::vec3 cameraPos;
    glm::vec3 cameraFront;
    glm::vec3 cameraUp;

    // camera Attributes auto
    glm::vec3 cameraPosAuto;
    glm::vec3 cameraFrontAuto;
    glm::vec3 cameraUpAuto;

    // const glm::vec3 front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);

    float yawAuto{};
    float pitchAuto{};

    float mouseSensitivity = 0.3f;

    float movementSpeed = 30.0f;

    float time=0.0f;
    bool automatic = false;

    CameraMovement *movement;
    bool reloadMovement;
};