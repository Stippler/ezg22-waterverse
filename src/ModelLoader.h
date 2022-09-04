#pragma once

#include <functional>

#include "Model.h"

namespace ModelLoader
{
    void addModel(std::string path);
    void free();
    void renderAll(std::function<void(Model *model)> callback);
}