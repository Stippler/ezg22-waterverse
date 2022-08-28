#include "FileWatcher.h"

#include <thread>
#include <unordered_map>
#include <iostream>
#include <filesystem>

std::mutex addShaderMutex{};

struct FileInfo
{
    std::filesystem::path path;
    std::filesystem::file_time_type lastWriteTime;
};

std::unordered_map<std::string, FileInfo> files;

void watchFiles(std::stop_token stoken)
{
    auto delay = std::chrono::milliseconds(100);

    while (!stoken.stop_requested())
    {
        std::this_thread::sleep_for(delay);
        addShaderMutex.lock();
        for (auto &pair : files)
        {
            auto info = pair.second;
            auto currentFileLastWriteTime = std::filesystem::last_write_time(info.path);
            if (currentFileLastWriteTime != info.lastWriteTime)
            {
                // TODO
            }
        }
        addShaderMutex.unlock();
    }
    std::cout << "Stopped file watcher" << std::endl;
}

void FileWatcher::start()
{
}

void FileWatcher::stop()
{
}

void FileWatcher::add(std::string file)
{
}