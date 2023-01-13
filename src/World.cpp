#include "World.h"

#include <glm/glm.hpp>
#include <assert.h>
#include <unordered_map>
#include <vector>

#include "AnimatedModel.h"
#include "Window.h"
#include "Water.h"
#include "FileWatcher.h"

std::vector<GameObject *> animatedObjects;
std::vector<GameObject *> staticObjects;

std::vector<float> swarmTimes;
std::vector<glm::vec3> swarmPositions;

std::vector<float> swarmTimes2;

std::unordered_map<std::string, AnimatedModel *> animatedModelMap;
std::unordered_map<std::string, AnimatedModel *> staticModelMap;

DirLight *light;
std::vector<PointLight *> plights;
std::vector<GameObject *> swarm;
std::vector<GameObject *> predators;
std::vector<GameObject *> spheres;
Water *water;

// Swarm update:
float collision_radius = 1.0f;
float view_radius = 3.0f;
float center_radius = 12.0f;
float predator_radius = 5.0f;

float collision_influence = 0.7f;
float velocity_influence = 0.15f;
float position_influence = 0.15f;
float center_influence = 2.0f;
float predator_influence = 0.3f;

float acceleration = 5.0f;
float max_speed = 5.0f;

int swarm_size = 50;
bool reload_world = false;

float worldTime = 0.0f;

// std::vector<AnimatedModel *> models;
// std::vector<Cube*> cubes;

void World::init()
{
    animatedModelMap.emplace("whiteshark", new AnimatedModel("assets/models/whiteshark/WhiteShark.gltf", glm::vec3(0, 0, 1.0f)));
    // animatedModelMap.emplace("fish", new AnimatedModel("assets/models/guppy-fish/Guppy.gltf", glm::vec3(0, 0, -1.0f)));
    animatedModelMap.emplace("fish", new AnimatedModel("assets/models/lowpoly-fish/lowfish.gltf", glm::vec3(0.0f, -1.0f, 0.0f)));
    // animatedModelMap.emplace("manta", new AnimatedModel("assets/models/manta/scene.gltf", glm::vec3(0, -1.0f, 0)));
    staticModelMap.emplace("crate", new AnimatedModel("assets/models/Crate/Crate1.obj"));
    staticModelMap.emplace("ground", new AnimatedModel("assets/models/floor/floor.obj"));
    staticModelMap.emplace("sphere", new AnimatedModel("assets/models/sphere/sphere.obj"));

    water = new Water();
    light = new DirLight(glm::vec3(0.01f, -1.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f));

    // light = new DirLight(glm::vec3(1.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    plights.push_back(new PointLight(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.5, 1.0));
    // plights.push_back(new PointLight(glm::vec3(3.0f, -8.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.5, 1.0));
    // plights.push_back(new PointLight(glm::vec3(2.0f, -4.0f, 0.0f), glm::vec3(0.2f, 0.2f, 0.2f), glm::vec3(0.5f, 0.5f, 0.5f), glm::vec3(1.0f, 1.0f, 1.0f), 1.0, 0.5, 1.0));

    World::reload();
    FileWatcher::add("assets/swarm.txt", [&]
                     { reload_world = true; });
}

float World::getTime()
{
    return worldTime;
}

float read_num(std::ifstream &file)
{
    std::string dummy_string;
    char dummy_char;
    float num;
    file >> num;
    file >> dummy_char;
    file >> dummy_string;
    file >> dummy_char;
    file >> dummy_string;
    return num;
}

void World::reload()
{
    std::ifstream file;
    file.open("assets/swarm.txt");

    collision_radius = read_num(file);
    view_radius = read_num(file);
    center_radius = read_num(file);
    predator_radius = read_num(file);
    collision_influence = read_num(file);
    velocity_influence = read_num(file);
    position_influence = read_num(file);
    center_influence = read_num(file);
    predator_influence = read_num(file);
    acceleration = read_num(file);
    max_speed = read_num(file);
    swarm_size = read_num(file);

    file.close();

    std::cout << "----------------" << std::endl;
    std::cout << "Reload World" << std::endl;
    std::cout << "collision_radius " << collision_radius
              << std::endl
              << "view_radius " << view_radius
              << std::endl
              << "center_radius " << center_radius
              << std::endl
              << "collision_influence " << collision_influence
              << std::endl
              << "velocity_influence " << velocity_influence
              << std::endl
              << "position_influence " << position_influence
              << std::endl
              << "center_influence " << center_influence
              << std::endl
              << "acceleration " << acceleration
              << std::endl
              << "max_speed " << max_speed
              << std::endl
              << "swarm_size " << swarm_size
              << std::endl;
    std::cout << "----------------" << std::endl;

    World::clear();
    // World::addGameObject("sphere", glm::vec3(0, 3, 0));
    auto go = World::addGameObject("whiteshark", true, glm::vec3(0, -8, 0), 0.4f);
    go->velocity = glm::vec3(1, 0, 1);
    predators.push_back(go);
    // auto sphere = World::addGameObject("sphere", glm::vec3(0, 1, 0));
    // auto manta = World::addGameObject("manta", glm::vec3(0, -8, 5), 2.0f);
    // spheres.push_back(sphere);

    float gridSize = 5;

    for (int i = 0; i < swarm_size; i++)
    {
        float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5;
        float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5;
        float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) - 0.5;
        GameObject *fish = World::addGameObject("fish", false, glm::vec3(x * gridSize, y * gridSize, z * gridSize), .2f);
        swarm.push_back(fish);
    }
    World::addGameObject("ground", false, glm::vec3(0, -15 + 0.966666, 0), 0.9666666);
}

void World::renderGameObjects(Shader *shader)
{
    shader->use();
    Window::setMatrices(shader);

    for (auto go : World::getStaticObjects())
    {
        if (go->animated == true)
        {
            shader->setInt("animated", 1);
        }
        else
        {
            shader->setInt("animated", 0);
        }
        go->render(shader);
    }

    for (auto go : World::getAnimatedObjects())
    {
        if (go->animated == true)
        {
            shader->setInt("animated", 1);
        }
        else
        {
            shader->setInt("animated", 0);
        }
        go->render(shader);
    }
}

void World::renderSpheres(Shader *shader)
{
    for (auto sphere : spheres)
    {
        sphere->render(shader);
    }
}

void World::renderWater(Shader *waterShader)
{
    water->render(waterShader);
}

void World::renderCaustics(unsigned int environment)
{
    water->renderCaustics(environment);
}

void World::update(float tslf)
{
    worldTime += tslf;
    if (reload_world)
    {
        World::reload();
        reload_world = false;
    }
    Window::getCamera()->update(tslf);
    water->update(tslf);

    for (auto predator : predators)
    {
        glm::vec3 dir = glm::normalize(predator->velocity) * 0.4f;
        //if (glm::length(predator->pos) > 13)
        //{
        dir += glm::normalize(-predator->pos) * 0.5f * glm::length(predator->pos)/14.0f;
        glm::vec3 updown=glm::vec3(0, -predator->pos.y, 0);
        dir +=  glm::normalize(updown)* 0.2f  * glm::length(updown);
        //}
        float r1 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float r2 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        float r3 = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
        dir += glm::normalize(glm::vec3(r1, r2, r3)) * 0.1f;

        dir = glm::normalize(dir);
        predator->velocity += 5.0f * dir * tslf;

        float speed = glm::length(predator->velocity);
        if (speed > 5)
        {
            predator->velocity *= (5 / speed);
        }
    }

    for (auto curr_obj : swarm)
    {
        // calculate average position and speed:
        int view_count = 0;
        int collision_count = 0;
        glm::vec3 center = glm::vec3(0);
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 velocity = glm::vec3(0.0f);
        glm::vec3 collision_position = glm::vec3(0.0f);
        for (auto neighbour : swarm)
        {
            if (neighbour == curr_obj)
            {
                continue;
            }
            float dist = glm::length(neighbour->pos - curr_obj->pos);
            if (dist > view_radius)
            {
                continue;
            }
            position += neighbour->pos;
            velocity += neighbour->velocity;
            view_count++;
            if (dist > collision_radius)
            {
                continue;
            }
            collision_position += neighbour->pos;
            collision_count++;
        }

        glm::vec3 dir = glm::vec3(0);
        if (view_count != 0)
        {
            position /= view_count;
            velocity /= view_count;

            if (glm::length(position) != 0)
            {
                glm::vec3 position_dir = glm::normalize(position - curr_obj->pos);
                dir += position_dir * position_influence;
            }

            if (glm::length(velocity) != 0)
            {
                glm::vec3 velocity_dir = glm::normalize(velocity);
                dir += velocity_dir * velocity_influence;
            }
        }

        if (collision_count != 0)
        {
            collision_position /= collision_count;
            if (glm::length(collision_position) != 0)
            {
                glm::vec3 collision_dir = glm::normalize(curr_obj->pos - collision_position);
                dir += collision_dir * collision_influence;
            }
        }

        float dist_to_center = glm::length(curr_obj->pos.x);
        float x = curr_obj->pos.x;
        float y = curr_obj->pos.y;
        float z = curr_obj->pos.z;
        if (x > center_radius)
        {
            dir += glm::vec3(-1, 0, 0) * center_influence;
        }
        if (x < -center_radius)
        {
            dir += glm::vec3(1, 0, 0) * center_influence;
        }
        if (y > center_radius)
        {
            dir += glm::vec3(0, -1, 0) * center_influence;
        }
        if (y < -center_radius)
        {
            dir += glm::vec3(0, 1, 0) * center_influence;
        }
        if (z > center_radius)
        {
            dir += glm::vec3(0, 0, -1) * center_influence;
        }
        if (z < -center_radius)
        {
            dir += glm::vec3(0, 0, 1) * center_influence;
        }

        glm::vec3 predator_dir = curr_obj->pos - predators[0]->pos;
        if (glm::length(predator_dir) < predator_radius)
        {
            dir += glm::normalize(predator_dir) * predator_influence;
        }

        if (glm::length(dir) == 0)
        {
            dir = glm::vec3(0, 0, 1.0f);
        }
        dir = glm::normalize(dir);

        curr_obj->velocity += acceleration * dir * tslf;

        float speed = glm::length(curr_obj->velocity);
        if (speed > max_speed)
        {
            curr_obj->velocity *= (max_speed / speed);
        }
    }
    for (auto ao : animatedObjects)
    {
        ao->pos += ao->velocity * tslf;
        ao->time += tslf;
    }
    // swarm update end
}

void World::clear()
{
    for (auto obj : staticObjects)
    {
        delete obj;
    }
    for (auto obj : animatedObjects)
    {
        delete obj;
    }
    for (auto obj : spheres)
    {
        delete obj;
    }

    staticObjects.clear();
    animatedObjects.clear();
    spheres.clear();
    swarm.clear();
    predators.clear();
}

GameObject *World::addGameObject(std::string model, bool animated, glm::vec3 pos, float scale)
{
    if (staticModelMap.find(model) != staticModelMap.end())
    {
        GameObject *gameObject = new GameObject(staticModelMap.at(model), animated);
        gameObject->pos = pos;
        gameObject->scale = scale;
        staticObjects.push_back(gameObject);
        return gameObject;
    }
    else if (animatedModelMap.find(model) != animatedModelMap.end())
    {
        GameObject *gameObject = new GameObject(animatedModelMap.at(model), animated);
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

    float near_plane = 1.0f, far_plane = 30.0f;
    lightProjection = glm::ortho(-18.0f, 18.0f, -18.0f, 18.0f, near_plane, far_plane);
    lightView = glm::lookAt(-2.0f * light->direction, glm::vec3(0.0f, -0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
    lightSpaceMatrix = lightProjection * lightView;

    return lightSpaceMatrix;
}

std::vector<GameObject *> World::getSpheres()
{
    return spheres;
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

unsigned int World::getCaustics()
{
    return water->caustics;
}