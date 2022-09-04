#include <mutex>
#include <unordered_map>
#include <thread>

#include "ModelLoader.h"
#include "FileWatcher.h"
#include "SafeQueue.h"

std::mutex addModelMutex{};
std::unordered_map<std::string, Model *> models{};

SafeQueue<Model*> loadedModels;


void loadModels(std::stop_token stoken) {
    while (!stoken.stop_requested())
    {
        std::string path = "ledl";
        Model *model = new Model(path);

        addModelMutex.lock();
        auto it = models.find(path);
        if (it != models.end())
        {
            Model *oldModel = it->second;
            delete oldModel;
            it->second = model;
            std::cout << "reloaded " << path << std::endl;
        }
        else
        {
            models.emplace(path, model);
            std::cout << "loaded " << path << std::endl;
        }
        addModelMutex.unlock();
    }
}

void ModelLoader::addModel(std::string path)
{
    assert(models.find(path) == models.end());

    // TODO: bool gamma
    

    // TODO: discuss this hack add file to FileWatcher
    // std::thread t1(callback);
    // t1.detach();

    // FileWatcher::add(path, callback);
}

void ModelLoader::free() {
    for (auto &pair : models)
    {
        delete pair.second;
    }
}

void ModelLoader::renderAll(std::function<void(Model *model)> callback)
{
    addModelMutex.lock();
    for (auto &pair : models)
    {
        callback(pair.second);
    }
    addModelMutex.unlock();
}