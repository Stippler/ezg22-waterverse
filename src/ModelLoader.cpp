#include <mutex>
#include <unordered_map>
#include <thread>

#include "ModelLoader.h"
#include "FileWatcher.h"
#include "SafeQueue.h"

std::mutex addModelMutex{};
std::unordered_map<std::string, Model *> models{};
std::jthread modelLoader;

SafeQueue<std::string> pathQueue;

void loadModels(std::stop_token stoken)
{
    while (!stoken.stop_requested())
    {
        std::string path;
        if(pathQueue.ConsumeSync(path)){
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
}

void ModelLoader::addModel(std::string path)
{
    assert(models.find(path) == models.end());

    pathQueue.Produce(std::move(std::string(path)));
    FileWatcher::add(path, [=](){
        pathQueue.Produce(std::move(std::string(path)));
    });
}

void ModelLoader::start()
{
    modelLoader = std::jthread(loadModels);
}

void ModelLoader::stop()
{
    modelLoader.request_stop();
    modelLoader.join();
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