#pragma once

#include "Camera.h"

namespace Window {
    void init();
    void processInput();
    void swapBuffers();
    void cleanup();

    float getWidth();
    float getHeight();

    bool shouldClose();
    Camera* getCamera();
}