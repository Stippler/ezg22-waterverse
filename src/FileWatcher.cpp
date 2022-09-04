#include "FileWatcher.h"

#include <thread>
#include <unordered_map>
#include <iostream>
#include <filesystem>
#include <exception>

std::mutex addShaderMutex{};
std::jthread fileWatcher;

struct FileInfo
{
    std::filesystem::path path;
    std::filesystem::file_time_type lastWriteTime;
    std::function<void()> callback;

    FileInfo(std::string file, std::function<void()> callback) : path(file), callback(callback)
    {
        if(!std::filesystem::exists(file)){
            std::cerr << "File does not exist: " << file << std::endl;
            throw std::exception();
        }
        lastWriteTime = std::filesystem::last_write_time(path);
    }
};

std::vector<FileInfo *> files;

void watchFiles(std::stop_token stoken)
{
    auto delay = std::chrono::milliseconds(100);

    while (!stoken.stop_requested())
    {
        std::this_thread::sleep_for(delay);
        addShaderMutex.lock();
        for (auto &file : files)
        {
            auto info = file;
            auto currentFileLastWriteTime = std::filesystem::last_write_time(info->path);
            if (currentFileLastWriteTime != info->lastWriteTime)
            {
                file->callback();
                file->lastWriteTime = currentFileLastWriteTime;
            }
        }
        addShaderMutex.unlock();
    }
}

void FileWatcher::start()
{
    fileWatcher = std::jthread(watchFiles);
}

void FileWatcher::stop()
{
    fileWatcher.request_stop();
    fileWatcher.join();

    for (auto info : files)
    {
        delete info;
    }
}

void FileWatcher::add(std::string file, std::function<void()> callback)
{
    FileInfo *info = new FileInfo(file, callback);
    addShaderMutex.lock();
    files.push_back(info);
    addShaderMutex.unlock();
}