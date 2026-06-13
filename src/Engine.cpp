#include "Engine.h"
#include "InputHandler.h"
#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <windows.h>
#include <commdlg.h>

Engine::Engine(int width, int height, const std::string& title)
    : window(nullptr), windowWidth(width), windowHeight(height), windowTitle(title),
      worldMinX(-100.0f), worldMaxX(100.0f), worldMinY(-100.0f), worldMaxY(100.0f),
      activeTool(TOOL_SELECT), activeShapeType(SHAPE_RECTANGLE), shapeSize(30.0f),
      showGrid(true), isDrawing(false), isPanning(false), isDragging(false),
      lastMouseX(0.0), lastMouseY(0.0), dragStartX(0.0f), dragStartY(0.0f), dragScaleFactor(1.0f), dragAngle(0.0f),
      isSelecting(false), selBoxX1(0.0f), selBoxY1(0.0f), selBoxX2(0.0f), selBoxY2(0.0f), snapToGridEnabled(true),
      panelWidth(195.0f), currentFilePath{0}, maxPolygonVerts(100),
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

// Per-frame update placeholder (continuous logic, animations, etc.)
void Engine::update(float deltaTime) {
    (void)deltaTime;
}

bool Engine::isSelected(int idx) const {
    for (int s : selectedShapes) if (s == idx) return true;
    return false;
}

void Engine::clearSelection() { selectedShapes.clear(); }

void Engine::selectSingle(int idx) {
    clearSelection();
    if (idx >= 0 && idx < (int)shapes.size())
        selectedShapes.push_back(idx);
}

void Engine::addToSelection(int idx) {
    if (idx >= 0 && idx < (int)shapes.size() && !isSelected(idx))
        selectedShapes.push_back(idx);
}

int Engine::firstSelected() const {
    return selectedShapes.empty() ? -1 : selectedShapes[0];
}

Point2D Engine::selectedCentroid() const {
    Point2D sum(0, 0, 0);
    for (int idx : selectedShapes) {
        Point2D c = shapes[idx].centroid();
        sum.x += c.x; sum.y += c.y;
    }
    if (!selectedShapes.empty()) {
        sum.x /= (float)selectedShapes.size();
        sum.y /= (float)selectedShapes.size();
    }
    return sum;
}

void Engine::pushUndo() {
    undoStack.push_back(shapes);
    if (undoStack.size() > 100) undoStack.erase(undoStack.begin());
    redoStack.clear();
}

void Engine::bringToFront() {
    if (selectedShapes.empty()) return;
    pushUndo();
    std::vector<Shape> saved;
    for (int idx : selectedShapes) saved.push_back(shapes[idx]);
    for (int i = (int)selectedShapes.size() - 1; i >= 0; i--)
        shapes.erase(shapes.begin() + selectedShapes[i]);
    selectedShapes.clear();
    for (auto& s : saved) {
        shapes.push_back(s);
        selectedShapes.push_back((int)shapes.size() - 1);
    }
}

void Engine::sendToBack() {
    if (selectedShapes.empty()) return;
    pushUndo();
    std::vector<Shape> saved;
    for (int idx : selectedShapes) saved.push_back(shapes[idx]);
    for (int i = (int)selectedShapes.size() - 1; i >= 0; i--)
        shapes.erase(shapes.begin() + selectedShapes[i]);
    selectedShapes.clear();
    for (int i = 0; i < (int)saved.size(); i++) {
        shapes.insert(shapes.begin() + i, saved[i]);
        selectedShapes.push_back(i);
    }
}

void Engine::copySelected() {
    if (selectedShapes.empty()) return;
    clipboard.clear();
    for (int idx : selectedShapes) clipboard.push_back(shapes[idx]);
}

void Engine::pasteClipboard() {
    if (clipboard.empty()) return;
    pushUndo();
    clearSelection();
    for (auto& s : clipboard) {
        Shape c = s;
        c.translate(10.0f, 10.0f);
        shapes.push_back(c);
        selectedShapes.push_back((int)shapes.size() - 1);
    }
}

void Engine::undo() {
    if (undoStack.empty()) return;
    redoStack.push_back(shapes);
    shapes = undoStack.back();
    undoStack.pop_back();
    clearSelection();
}

void Engine::redo() {
    if (redoStack.empty()) return;
    undoStack.push_back(shapes);
    shapes = redoStack.back();
    redoStack.pop_back();
    clearSelection();
}

void Engine::saveShapes(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "%zu\n", shapes.size());
    for (auto& s : shapes) {
        fprintf(f, "%zu %d\n", s.vertices.size(), s.closed ? 1 : 0);
        fprintf(f, "%f %f %f\n", s.fill[0], s.fill[1], s.fill[2]);
        fprintf(f, "%f %f %f\n", s.stroke[0], s.stroke[1], s.stroke[2]);
        for (auto& pt : s.vertices)
            fprintf(f, "%f %f\n", pt.x, pt.y);
    }
    fclose(f);
}

void Engine::loadShapes(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    size_t count;
    if (fscanf(f, "%zu", &count) != 1) { fclose(f); return; }
    shapes.clear();
    clearSelection();
    currentShape.clear();
    isDrawing = false;
    for (size_t i = 0; i < count; ++i) {
        Shape s;
        size_t vertCount;
        int closedVal;
        if (fscanf(f, "%zu %d", &vertCount, &closedVal) != 2) break;
        s.closed = (closedVal != 0);
        if (fscanf(f, "%f %f %f", &s.fill[0], &s.fill[1], &s.fill[2]) != 3) break;
        if (fscanf(f, "%f %f %f", &s.stroke[0], &s.stroke[1], &s.stroke[2]) != 3) break;
        s.vertices.resize(vertCount);
        for (size_t j = 0; j < vertCount; ++j) {
            if (fscanf(f, "%f %f", &s.vertices[j].x, &s.vertices[j].y) != 2) break;
            s.vertices[j].w = 1.0f;
        }
        shapes.push_back(s);
    }
    fclose(f);
}

void Engine::exportSVG(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;

    float minX = 0, maxX = 0, minY = 0, maxY = 0;
    bool first = true;
    for (auto& s : shapes) {
        for (auto& pt : s.vertices) {
            if (first) { minX = maxX = pt.x; minY = maxY = pt.y; first = false; }
            else { if (pt.x < minX) minX = pt.x; if (pt.x > maxX) maxX = pt.x; if (pt.y < minY) minY = pt.y; if (pt.y > maxY) maxY = pt.y; }
        }
    }
    float pad = 10.0f;
    minX -= pad; maxX += pad; minY -= pad; maxY += pad;

    fprintf(f, "<svg xmlns=\"http://www.w3.org/2000/svg\" viewBox=\"%f %f %f %f\">\n", minX, minY, maxX - minX, maxY - minY);
    for (auto& s : shapes) {
        fprintf(f, "  <%s points=\"", s.closed ? "polygon" : "polyline");
        for (size_t i = 0; i < s.vertices.size(); i++) {
            if (i > 0) fputc(' ', f);
            fprintf(f, "%f,%f", s.vertices[i].x, s.vertices[i].y);
        }
        fprintf(f, "\" fill=\"%s\" stroke=\"rgb(%d,%d,%d)\" stroke-width=\"0.5\"/>\n",
            s.closed ? "rgb(204,204,255)" : "none",
            (int)(s.stroke[0]*255), (int)(s.stroke[1]*255), (int)(s.stroke[2]*255));
    }
    fprintf(f, "</svg>\n");
    fclose(f);
}

void Engine::exportWithDialog() {
    OPENFILENAMEA ofn = {};
    char path[260] = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "SVG Files\0*.svg\0All Files\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "svg";
    if (GetSaveFileNameA(&ofn)) exportSVG(path);
}

void Engine::saveWithDialog() {
    OPENFILENAMEA ofn = {};
    char path[260] = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "CAD Files\0*.cad\0All Files\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt = "cad";
    if (GetSaveFileNameA(&ofn)) {
        strcpy(currentFilePath, path);
        saveShapes(path);
    }
}

void Engine::openWithDialog() {
    OPENFILENAMEA ofn = {};
    char path[260] = {};
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = nullptr;
    ofn.lpstrFilter = "CAD Files\0*.cad\0All Files\0*.*\0";
    ofn.lpstrFile = path;
    ofn.nMaxFile = sizeof(path);
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.lpstrDefExt = "cad";
    if (GetOpenFileNameA(&ofn)) {
        strcpy(currentFilePath, path);
        loadShapes(path);
    }
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
