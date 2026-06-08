#ifndef ENGINE_H
#define ENGINE_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include "../include/Transformations.h"

class Engine {
public:
    Engine(int width, int height, const std::string& title);
    ~Engine();

    bool initialize();
    void run();
    void shutdown();

    // World Window coordinates for panning/zooming
    void setWorldWindow(float minX, float maxX, float minY, float maxY);
    
    // Viewport coordinates
    void setViewport(int width, int height);

private:
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    std::string windowTitle;

    // Viewport and World coordinates
    float worldMinX, worldMaxX;
    float worldMinY, worldMaxY;

    // Shape storage (Member 4: populate, Member 3: read for selection)
    std::vector<std::vector<Point2D>> shapes;

    // Rendering loop
    void update(float deltaTime);
    void render();
    void scanLineFill(const std::vector<Point2D>& polygon);

    // GLFW Callbacks
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
};

#endif // ENGINE_H
