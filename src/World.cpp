#include "World.h"
#include "AnimatedModel.h"
#include "Window.h"

#include <assert.h>
#include <unordered_map>

std::vector<GameObject *> animatedObjects;
std::vector<GameObject *> staticObjects;

std::unordered_map<std::string, AnimatedModel *> animatedModelMap;
std::unordered_map<std::string, AnimatedModel *> staticModelMap;

// std::vector<AnimatedModel *> models;
// std::vector<Cube*> cubes;

void World::init()
{
    animatedModelMap.emplace("whiteshark", new AnimatedModel("assets/models/whiteshark/WhiteShark.gltf"));
    animatedModelMap.emplace("fish", new AnimatedModel("assets/models/guppy-fish/Guppy.gltf"));
    staticModelMap.emplace("crate", new AnimatedModel("assets/models/Crate/Crate1.obj"));
    staticModelMap.emplace("ground", new AnimatedModel("assets/models/floor/floor.obj"));

    World::addGameObject("whiteshark", glm::vec3(0, -8, 0));
    World::addGameObject("fish", glm::vec3(2, -4, 0));
    World::addGameObject("fish", glm::vec3(0, -4, 0));
    World::addGameObject("fish", glm::vec3(-2, -4, 0));
    World::addGameObject("ground", glm::vec3(0, -15, 0));
    World::addGameObject("crate", glm::vec3(0, 5, 0));
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
        ao->time += tslf;
    }
}

GameObject *World::addGameObject(std::string model, glm::vec3 pos)
{
    if (staticModelMap.find(model) != staticModelMap.end())
    {
        GameObject *gameObject = new GameObject(staticModelMap.at(model), false);
        gameObject->pos = pos;
        staticObjects.push_back(gameObject);
        return gameObject;
    }
    else if (animatedModelMap.find(model) != animatedModelMap.end())
    {
        GameObject *gameObject = new GameObject(animatedModelMap.at(model), true);
        gameObject->pos = pos;
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

std::vector<GameObject *> World::getAnimatedObjects()
{
    return animatedObjects;
}

std::vector<GameObject *> World::getStaticObjects()
{
    return staticObjects;
}