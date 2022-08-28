#include "Window.h"

#include <GLFW/glfw3.h>
#include <exception>
#include <iostream>

GLFWwindow *window = nullptr;
static Camera *camera = nullptr;
static bool mouseEnabled;
static double lastX = 400;
static double lastY = 300;
bool wireframe = false;
int width = 800;
int height = 600;

// Callbacks
void mouseMoveCallback(GLFWwindow *window, double xpos, double ypos)
{
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;
    if (mouseEnabled)
    {
        // const float sensitivity = 10.f;
        camera->processMouseMovement(xoffset, yoffset);
    }
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods)
{
    int state = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
    if (state == GLFW_PRESS)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if (glfwRawMouseMotionSupported())
        {
            glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
        }

        glfwGetCursorPos(window, &lastX, &lastY);
        // set global camera for callback
        mouseEnabled = true;
    }
    if (state == GLFW_RELEASE)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        mouseEnabled = false;
    }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Window::processInput()
{
    glfwPollEvents();

    float currentFrame = glfwGetTime();
    static float lastFrame = glfwGetTime();
    float deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // throttle to 30 fps
    if (deltaTime > 1.0f / 30.0f)
    {
        //std::cout << "Info: input polled less than 30 times per second" << std::endl;
        deltaTime = 1.0f / 30.0f;
    }

    if (camera)
    {
        auto glfWwindow = window;
        // TODO handle input for camera
        if (glfwGetKey(glfWwindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            camera->moveForward(deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            camera->moveBackward(deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            camera->moveLeft(deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            camera->moveRight(deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            camera->moveUp(deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            camera->moveDown(deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_UP) == GLFW_PRESS)
        {
            camera->processMouseMovement(0, 180.f * 2 * deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_DOWN) == GLFW_PRESS)
        {
            camera->processMouseMovement(0, -180.f * 2 * deltaTime);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_LEFT) == GLFW_PRESS)
        {
            camera->processMouseMovement(-360.f * 2 * deltaTime, 0);
        }
        if (glfwGetKey(glfWwindow, GLFW_KEY_RIGHT) == GLFW_PRESS)
        {
            camera->processMouseMovement(360.f * 2 * deltaTime, 0);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_F1) == GLFW_PRESS)
    {
        wireframe = !wireframe;
        if (!wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else if (wireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }
}

void Window::init()
{
    if (camera != nullptr)
    {
        // init has been called twice
        throw std::exception();
    }
    camera = new Camera(-2, 0, 0);

    // initialize glfw and set hints
    if (!glfwInit())
    {
        std::cerr << "Error occured when initializing glfw" << std::endl;
        throw std::exception();
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create a glfw window
    window = glfwCreateWindow(800, 600, "Waterverse", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        throw std::exception();
    }

    // set created window as current context
    glfwMakeContextCurrent(window);

    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, mouseMoveCallback);

    // set viewport
    glViewport(0, 0, width, height);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void Window::swapBuffers()
{
    glfwSwapBuffers(window);
}

void Window::cleanup()
{
    delete camera;

    // terminate glfw after done
    glfwTerminate();
}

float Window::getWidth()
{
    return width;
}

float Window::getHeight()
{
    return height;
}

bool Window::shouldClose()
{
    return glfwWindowShouldClose(window);
}

Camera *Window::getCamera()
{
    return camera;
}