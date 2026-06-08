#include "InputHandler.h"
#include "Engine.h"
#include "Clipping.h"
#include "Transformations.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"

Engine* InputHandler::engine = nullptr;

void InputHandler::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (e) e->setViewport(width, height);
}

// ============================================================
// MEMBER 3 TODO: Implement cursorPositionCallback
// Handle: canvas panning (isPanning), shape dragging (isDragging)
// for Move/Rotate/Scale tools, freehand pen drawing (TOOL_PEN).
// Update lastMouseX/lastMouseY for tracking.
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e) return;

    // TODO: handle panning (e->isPanning), dragging (e->isDragging), pen drawing

    e->lastMouseX = xpos;
    e->lastMouseY = ypos;
}

// ============================================================
// MEMBER 3 TODO: Implement mouseButtonCallback
// Handle all tools: SELECT, DRAW_POLYGON, DRAW_LINE, PEN, FILL,
// MOVE, ROTATE, SCALE, SHAPE.
// Use screenToWorld to get world coords. Use isPointInPolygon
// via Shape::contains() for hit-testing.
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e) return;

    if (ImGui::GetIO().WantCaptureMouse) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float wx, wy;
    e->renderer.screenToWorld(xpos, ypos, wx, wy);

    // TODO: dispatch per-tool logic in switch(e->activeTool)

    (void)button; (void)action; (void)mods; (void)wx; (void)wy;
}

// ============================================================
// MEMBER 3 TODO: Implement scrollCallback
// Zoom in/out around world center. Scroll up = zoom in (0.9x),
// scroll down = zoom out (1.1x). Modify worldMin/Max accordingly.
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

    // TODO: modify engine->worldMinX/Y, worldMaxX/Y based on scroll direction
    (void)xoffset; (void)yoffset;
}

void InputHandler::windowFocusCallback(GLFWwindow* window, int focused) {
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
}

void InputHandler::cursorEnterCallback(GLFWwindow* window, int entered) {
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
}

// ============================================================
// MEMBER 3 TODO: Implement keyCallback
// Handle tool hotkeys (1-5, M, R, S), modifier+key combos (Ctrl+Z/Y/S/O),
// Delete/Backspace to remove selected shape, Esc to finish drawing or close app.
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e || ImGui::GetIO().WantCaptureKeyboard) return;

    // TODO: keyboard shortcuts, Esc to finish drawing or close window
    (void)key; (void)scancode; (void)action; (void)mods;
}
