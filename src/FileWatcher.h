#pragma once

// TODO: maybe use forward refs for faster compile time?
#include <string>
#include <vector>
#include <functional>

namespace FileWatcher
{
    void start();
    void add(std::string path, std::function<void()> callback);
    // returns and locks shader module
    // vk::ShaderModule getShaderModule(std::string key);
    // void freeShaderModule(std::string key);

    void stop();
}