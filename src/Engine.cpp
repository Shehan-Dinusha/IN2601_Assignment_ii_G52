#include "Engine.h"
#include "InputHandler.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>

Engine::Engine(int width, int height, const std::string& title)
    : window(nullptr), windowWidth(width), windowHeight(height), windowTitle(title),
      worldMinX(-100.0f), worldMaxX(100.0f), worldMinY(-100.0f), worldMaxY(100.0f),
      selectedShape(nullptr),
      activeTool(TOOL_SELECT), activeShapeType(SHAPE_RECTANGLE), shapeSize(30.0f),
      showGrid(true), isDrawing(false), isPanning(false), isDragging(false),
      lastMouseX(0.0), lastMouseY(0.0), dragStartX(0.0f), dragStartY(0.0f), dragScaleFactor(1.0f), dragAngle(0.0f),
      renderer(*this), uiBuilder(*this) {
    fillColor[0] = 0.6f; fillColor[1] = 0.6f; fillColor[2] = 1.0f;
    strokeColor[0] = 1.0f; strokeColor[1] = 1.0f; strokeColor[2] = 1.0f;
    InputHandler::engine = this;
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

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

    glfwSetFramebufferSizeCallback(window, InputHandler::framebufferSizeCallback);
    glfwSetCursorPosCallback(window, InputHandler::cursorPositionCallback);
    glfwSetMouseButtonCallback(window, InputHandler::mouseButtonCallback);
    glfwSetScrollCallback(window, InputHandler::scrollCallback);
    glfwSetWindowFocusCallback(window, InputHandler::windowFocusCallback);
    glfwSetCursorEnterCallback(window, InputHandler::cursorEnterCallback);
    glfwSetKeyCallback(window, InputHandler::keyCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }

    setViewport(windowWidth, windowHeight);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, false);
    ImGui_ImplOpenGL3_Init("#version 330");

    return true;
}

void Engine::run() {
    float lastFrameTime = 0.0f;

    while (!glfwWindowShouldClose(window)) {
        float currentFrameTime = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrameTime - lastFrameTime;
        lastFrameTime = currentFrameTime;

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        update(deltaTime);
        renderer.render();
        uiBuilder.buildUI();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Engine::update(float deltaTime) {
    (void)deltaTime;
}

void Engine::shutdown() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

void Engine::setWorldWindow(float minX, float maxX, float minY, float maxY) {
    worldMinX = minX; worldMaxX = maxX;
    worldMinY = minY; worldMaxY = maxY;
}

void Engine::setViewport(int width, int height) {
    windowWidth = width;
    windowHeight = height;
    glViewport(0, 0, width, height);
}
