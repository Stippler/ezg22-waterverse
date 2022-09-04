#pragma once

#include "Model.h"

namespace ModelLoader
{
    void start();
    void stop();

    void addModel(std::string path);
    void renderAll(std::function<void(Model *model)> callback);
}