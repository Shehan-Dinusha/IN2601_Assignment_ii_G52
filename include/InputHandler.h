#ifndef INPUTHANDLER_H
#define INPUTHANDLER_H

struct GLFWwindow;
class Engine;

class InputHandler {
public:
    static Engine* engine;

    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void windowFocusCallback(GLFWwindow* window, int focused);
    static void cursorEnterCallback(GLFWwindow* window, int entered);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
};

#endif
