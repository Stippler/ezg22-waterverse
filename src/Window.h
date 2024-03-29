#pragma once

#include "Camera.h"
#include "Shader.h"

namespace Window {
    void init();
    void processInput();
    void swapBuffers();
    void cleanup();

    void setMatrices(Shader *shader);

    void setScreenSize(Shader *shader);
    void setFullscreen(bool fullscreen);

    float getWidth();
    float getHeight();

    bool shouldClose();
    Camera* getCamera();
}