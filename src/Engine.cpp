#include "../include/Engine.h"
#include <iostream>

Engine::Engine(int width, int height, const std::string& title)
    : window(nullptr), windowWidth(width), windowHeight(height), windowTitle(title),
      worldMinX(-1.0f), worldMaxX(1.0f), worldMinY(-1.0f), worldMaxY(1.0f) {
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW for OpenGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

    window = glfwCreateWindow(windowWidth, windowHeight, windowTitle.c_str(), nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);

    // Set callbacks
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
    glfwSetCursorPosCallback(window, cursorPositionCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetScrollCallback(window, scrollCallback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    // Set initial viewport
    setViewport(windowWidth, windowHeight);

    return true;
}

void Engine::run() {
    float lastFrameTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        // Process inputs
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            glfwSetWindowShouldClose(window, true);

        update(deltaTime);
        render();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Engine::update(float deltaTime) {
    // TODO: Implement application logic update (e.g., animations, state changes)
}

void Engine::render() {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // TODO: Implement rendering of the CAD shapes using world and viewport coordinates
}

void Engine::scanLineFill(const std::vector<Point2D>& polygon) {
    // TODO: Implement scan-line polygon fill algorithm (Member 4)
    // 1. Find yMin and yMax of the polygon
    // 2. For each scan line y from yMin to yMax:
    //    a. Find all intersections of the scan line with polygon edges
    //    b. Sort intersections by x
    //    c. Draw horizontal segments between even-odd pairs using glBegin(GL_LINES)
}

void Engine::shutdown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Engine::setWorldWindow(float minX, float maxX, float minY, float maxY) {
    worldMinX = minX;
    worldMaxX = maxX;
    worldMinY = minY;
    worldMaxY = maxY;
}

void Engine::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}

void Engine::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        engine->setViewport(width, height);
    }
}

void Engine::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        // TODO: Handle mouse movement (e.g., panning, drawing line preview)
    }
}

void Engine::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        // TODO: Handle mouse clicks (e.g., creating points, selecting shapes)
    }
}

void Engine::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (engine) {
        // TODO: Handle scroll for zooming (update world window size/scale)
    }
}
