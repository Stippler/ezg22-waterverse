#include "World.h"
#include "AnimatedModel.h"
#include "Window.h"

#include <glm/glm.hpp>
#include <assert.h>
#include <unordered_map>
#include <vector>

std::vector<GameObject *> animatedObjects;
std::vector<GameObject *> staticObjects;

std::unordered_map<std::string, AnimatedModel *> animatedModelMap;
std::unordered_map<std::string, AnimatedModel *> staticModelMap;

DirLight *light;
std::vector<PointLight *> plights;

// std::vector<AnimatedModel *> models;
// std::vector<Cube*> cubes;

void World::init()
{
    light = new DirLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));
    //light = new DirLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    plights.push_back(new PointLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.5, 1.0));
    //plights.push_back(new PointLight(glm::vec3(3.0f, -8.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.5, 1.0));
    //plights.push_back(new PointLight(glm::vec3(2.0f, -4.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.5, 1.0));

    animatedModelMap.emplace("whiteshark", new AnimatedModel("assets/models/whiteshark/WhiteShark.gltf"));
    animatedModelMap.emplace("fish", new AnimatedModel("assets/models/guppy-fish/Guppy.gltf", M_PI));
    staticModelMap.emplace("crate", new AnimatedModel("assets/models/Crate/Crate1.obj"));
    staticModelMap.emplace("ground", new AnimatedModel("assets/models/floor/floor.obj"));

    auto go = World::addGameObject("whiteshark", glm::vec3(0, -8, 0));
    go->velocity = glm::vec3(0.5f, 0.5f, 0.5f);
    go = World::addGameObject("fish", glm::vec3(0, -4, 0), 0.1f);
    go->velocity = glm::vec3(0.5f, 0.0f, 0.0f);
    /*
    float gridSize = 10;

    for (int i = 0; i < 20; i++)
    {
        float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5;
        float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5;
        float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5;

        go = World::addGameObject("fish", glm::vec3(x * gridSize, y * gridSize, z * gridSize), .1f);
        go->velocity=glm::vec3(0.1f, 0.1f, 0.0f);
    }*/
    World::addGameObject("crate", glm::vec3(0, 5, 0));
    World::addGameObject("ground", glm::vec3(0, -15, 0));
}

void World::render(Shader *shader)
{
    shader->use();
    Window::setMatrices(shader);
    shader->setInt("animated", 1);
    for (auto go : World::getAnimatedObjects())
    {
        go->render(shader);
    }
    shader->setInt("animated", 0);
    for (auto go : World::getStaticObjects())
    {
        go->render(shader);
    }
}

void World::update(float tslf)
{
    for (auto ao : animatedObjects)
    {
        ao->pos += ao->velocity*tslf;
        ao->time += tslf;
    }
}

GameObject *World::addGameObject(std::string model, glm::vec3 pos, float scale)
{
    if (staticModelMap.find(model) != staticModelMap.end())
    {
        GameObject *gameObject = new GameObject(staticModelMap.at(model), false);
        gameObject->pos = pos;
        gameObject->scale = scale;
        staticObjects.push_back(gameObject);
        return gameObject;
    }
    else if (animatedModelMap.find(model) != animatedModelMap.end())
    {
        GameObject *gameObject = new GameObject(animatedModelMap.at(model), true);
        gameObject->pos = pos;
        gameObject->scale = scale;
        animatedObjects.push_back(gameObject);
        return gameObject;
    }
    else
    {
        std::cout << "Model name: '" << model << "' not found!" << std::endl;
        return nullptr;
    }
}

void World::free()
{
    for (const auto &p : animatedModelMap)
    {
        delete p.second;
    }
    for (const auto &p : staticModelMap)
    {
        delete p.second;
    }
    for (const auto &ao : animatedObjects)
    {
        delete ao;
    }
    for (const auto &so : staticObjects)
    {
        delete so;
    }
}

glm::mat4 World::getLightSpaceMatrix()
{
    // Render depth of scene to texture (from light's perspective)
    glm::mat4 lightProjection, lightView;
    glm::mat4 lightSpaceMatrix;

    float near_plane = 1.0f, far_plane = 35.0f;
    lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, near_plane, far_plane);
    lightView = glm::lookAt(-10.0f * light->direction, glm::vec3(0.0f, -0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    return lightSpaceMatrix;
}

std::vector<GameObject *> World::getAnimatedObjects()
{
    return animatedObjects;
}

std::vector<GameObject *> World::getStaticObjects()
{
    return staticObjects;
}

DirLight *World::getDirLight()
{
    return light;
}

std::vector<PointLight *> World::getPointLight()
{
    return plights;
}