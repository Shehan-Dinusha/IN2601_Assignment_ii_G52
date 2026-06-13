#include "InputHandler.h"
#include "Engine.h"
#include "Clipping.h"
#include "Transformations.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include <cmath>
#include <algorithm>

Engine* InputHandler::engine = nullptr;

void InputHandler::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (e) e->setViewport(width, height);
}

// Handle mouse movement for panning, dragging, pen drawing, and selection box
void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e) return;

    if (e->isPanning) {
        double dx = (e->lastMouseX - xpos) / e->windowWidth;
        double dy = (ypos - e->lastMouseY) / e->windowHeight;

        float rx = e->worldMaxX - e->worldMinX;
        float ry = e->worldMaxY - e->worldMinY;

        e->worldMinX += dx * rx;
        e->worldMaxX += dx * rx;
        e->worldMinY += dy * ry;
        e->worldMaxY += dy * ry;

        e->lastMouseX = xpos;
        e->lastMouseY = ypos;
        return;
    }

    if (e->isDragging && !e->selectedShapes.empty()) {
        float wx, wy;
        e->renderer.screenToWorld(xpos, ypos, wx, wy);

        if (e->activeTool == Engine::TOOL_MOVE) {
            float dx = wx - e->dragStartX;
            float dy = wy - e->dragStartY;

            for (int idx : e->selectedShapes)
                e->shapes[idx].translate(dx, dy);

            e->dragStartX = wx;
            e->dragStartY = wy;
        }
        else if (e->activeTool == Engine::TOOL_ROTATE) {
            float angle = static_cast<float>(xpos - e->lastMouseX) * 0.02f;

            Point2D c = e->selectedCentroid();
            for (int idx : e->selectedShapes)
                e->shapes[idx].rotate(angle, c);

            e->dragAngle += angle * 180.0f / 3.14159265f;
            if (e->dragAngle < 0.0f) e->dragAngle += 360.0f;
            if (e->dragAngle >= 360.0f) e->dragAngle -= 360.0f;
        }
        else if (e->activeTool == Engine::TOOL_ROTATE_ORIGIN) {
            float angle = static_cast<float>(xpos - e->lastMouseX) * 0.02f;

            for (int idx : e->selectedShapes)
                Transformations::rotateShapeAroundOrigin(e->shapes[idx].vertices, angle);

            e->dragAngle += angle * 180.0f / 3.14159265f;
            if (e->dragAngle < 0.0f) e->dragAngle += 360.0f;
            if (e->dragAngle >= 360.0f) e->dragAngle -= 360.0f;
        }
        else if (e->activeTool == Engine::TOOL_SCALE) {
            float s = 1.0f + static_cast<float>(xpos - e->lastMouseX) * 0.01f;

            Point2D c = e->selectedCentroid();

            Matrix3x3 t1 = Transformations::getTranslationMatrix(-c.x, -c.y);
            Matrix3x3 sc = Transformations::getScalingMatrix(s, s);
            Matrix3x3 t2 = Transformations::getTranslationMatrix(c.x, c.y);

            Matrix3x3 comp = t2 * sc * t1;

            for (int idx : e->selectedShapes)
                for (auto& pt : e->shapes[idx].vertices)
                    pt = comp * pt;

            e->dragScaleFactor *= s;
        }

        e->lastMouseX = xpos;
        e->lastMouseY = ypos;
        return;
    }

    if (e->isDrawing && e->activeTool == Engine::TOOL_PEN) {
        float wx, wy;
        e->renderer.screenToWorld(xpos, ypos, wx, wy);
        e->currentShape.push_back(Point2D(wx, wy, 1.0f));
    }

    if (e->isSelecting) {
        float wx, wy;
        e->renderer.screenToWorld(xpos, ypos, wx, wy);
        e->selBoxX2 = wx;
        e->selBoxY2 = wy;
    }

    e->lastMouseX = xpos;
    e->lastMouseY = ypos;
}

// Handle mouse clicks for all tools (Select, Draw, Fill, Transform, Place shape)
void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e) return;

    if (ImGui::GetIO().WantCaptureMouse) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);

    float wx, wy;
    e->renderer.screenToWorld(xpos, ypos, wx, wy);

    auto snapToExisting = [&](float& sx, float& sy) {
        if (e->snapToGridEnabled) {
            sx = std::round(wx / 10.0f) * 10.0f;
            sy = std::round(wy / 10.0f) * 10.0f;
        } else {
            sx = wx; sy = wy;
        }
        float bestDistSq = 25.0f; // threshold = 5 world units
        for (auto& shape : e->shapes) {
            for (auto& v : shape.vertices) {
                float dx = v.x - sx, dy = v.y - sy;
                float d = dx * dx + dy * dy;
                if (d < bestDistSq) { bestDistSq = d; sx = v.x; sy = v.y; }
            }
        }
    };

    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        e->isPanning = (action == GLFW_PRESS);
        return;
    }

    auto finishShape = [&]() {
        if (e->currentShape.size() >= 2) {
            Shape s;
            s.vertices = e->currentShape;
            s.closed = false;
            s.fill[0] = e->fillColor[0];
            s.fill[1] = e->fillColor[1];
            s.fill[2] = e->fillColor[2];
            s.stroke[0] = e->strokeColor[0];
            s.stroke[1] = e->strokeColor[1];
            s.stroke[2] = e->strokeColor[2];
            e->shapes.push_back(s);
        }

        e->isDrawing = false;
        e->currentShape.clear();
    };

    switch (e->activeTool) {
        case Engine::TOOL_SELECT:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->isSelecting = true;
                e->selBoxX1 = e->selBoxX2 = wx;
                e->selBoxY1 = e->selBoxY2 = wy;
                e->clearSelection();
                Point2D pt(wx, wy, 1.0f);

                for (int i = (int)e->shapes.size() - 1; i >= 0; i--) {
                    if (e->shapes[i].contains(pt)) {
                        e->selectSingle(i);
                        break;
                    }
                }
            } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                if (e->isSelecting) {
                    float dx = e->selBoxX2 - e->selBoxX1;
                    float dy = e->selBoxY2 - e->selBoxY1;
                    float minX = std::min(e->selBoxX1, e->selBoxX2);
                    float maxX = std::max(e->selBoxX1, e->selBoxX2);
                    float minY = std::min(e->selBoxY1, e->selBoxY2);
                    float maxY = std::max(e->selBoxY1, e->selBoxY2);
                    if (fabs(dx) > 1.0f || fabs(dy) > 1.0f) {
                        for (int i = (int)e->shapes.size() - 1; i >= 0; i--) {
                            Point2D c = e->shapes[i].centroid();
                            if (c.x >= minX && c.x <= maxX && c.y >= minY && c.y <= maxY) {
                                e->addToSelection(i);
                            }
                        }
                    }
                    e->isSelecting = false;
                }
            }
            break;

        case Engine::TOOL_DRAW_POLYGON:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                if ((int)e->currentShape.size() >= e->maxPolygonVerts) return;
                float sx, sy;
                snapToExisting(sx, sy);
                e->currentShape.push_back(Point2D(sx, sy, 1.0f));
                e->isDrawing = true;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                if (e->currentShape.size() >= 3) {
                    e->pushUndo();
                    Shape s;
                    s.vertices = e->currentShape;
                    s.closed = true;
                    s.fill[0] = e->fillColor[0];
                    s.fill[1] = e->fillColor[1];
                    s.fill[2] = e->fillColor[2];
                    s.stroke[0] = e->strokeColor[0];
                    s.stroke[1] = e->strokeColor[1];
                    s.stroke[2] = e->strokeColor[2];
                    e->shapes.push_back(s);
                }

                e->isDrawing = false;
                e->currentShape.clear();
            }
            break;

        case Engine::TOOL_DRAW_LINE:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                float sx, sy;
                snapToExisting(sx, sy);
                e->currentShape.push_back(Point2D(sx, sy, 1.0f));
                e->isDrawing = true;
            }
            else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                if (!e->currentShape.empty()) {
                    e->currentShape.pop_back();

                    if (e->currentShape.empty()) {
                        e->isDrawing = false;
                    }
                }
            }
            break;

        case Engine::TOOL_PEN:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->isDrawing = true;
                e->currentShape.clear();
                float sx, sy;
                snapToExisting(sx, sy);
                e->currentShape.push_back(Point2D(sx, sy, 1.0f));
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                e->pushUndo();
                finishShape();
            }
            break;

        case Engine::TOOL_FILL:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->pushUndo();
                Point2D pt(wx, wy, 1.0f);
                bool hit = false;

                for (int i = (int)e->shapes.size() - 1; i >= 0; i--) {
                    auto& s = e->shapes[i];
                    if (s.contains(pt)) {
                        s.fill[0] = e->fillColor[0];
                        s.fill[1] = e->fillColor[1];
                        s.fill[2] = e->fillColor[2];

                        s.stroke[0] = e->strokeColor[0];
                        s.stroke[1] = e->strokeColor[1];
                        s.stroke[2] = e->strokeColor[2];

                        hit = true;
                        break;
                    }
                }

                if (!hit) {
                    std::vector<std::vector<Point2D>> verts;
                    std::vector<bool> closed;

                    for (auto& s : e->shapes) {
                        verts.push_back(s.vertices);
                        closed.push_back(s.closed);
                    }

                    std::vector<Point2D> region;

                    if (Clipping::findEnclosedRegion(region, pt, verts, closed)) {
                        Shape s;
                        s.vertices = region;
                        s.closed = true;
                        s.fill[0] = e->fillColor[0];
                        s.fill[1] = e->fillColor[1];
                        s.fill[2] = e->fillColor[2];
                        s.stroke[0] = e->strokeColor[0];
                        s.stroke[1] = e->strokeColor[1];
                        s.stroke[2] = e->strokeColor[2];
                        e->shapes.push_back(s);
                    }
                }
            }
            break;

        case Engine::TOOL_MOVE:
        case Engine::TOOL_ROTATE:
        case Engine::TOOL_ROTATE_ORIGIN:
        case Engine::TOOL_SCALE:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->pushUndo();
                Point2D pt(wx, wy, 1.0f);
                int hitIdx = -1;

                for (int i = (int)e->shapes.size() - 1; i >= 0; i--) {
                    if (e->shapes[i].contains(pt)) { hitIdx = i; break; }
                }

                if (hitIdx >= 0) {
                    if (!e->isSelected(hitIdx))
                        e->selectSingle(hitIdx);
                    e->isDragging = true;
                    e->dragStartX = wx;
                    e->dragStartY = wy;
                    e->dragScaleFactor = 1.0f;

                    if (e->activeTool == Engine::TOOL_ROTATE ||
                        e->activeTool == Engine::TOOL_ROTATE_ORIGIN) {
                        e->dragAngle = 0.0f;
                    }
                } else {
                    e->clearSelection();
                }
            }
            else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                e->isDragging = false;
            }
            break;

        case Engine::TOOL_SHAPE:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->pushUndo();
                std::vector<Point2D> verts;
                float s = e->shapeSize;
                float px, py;
                snapToExisting(px, py);

                switch (e->activeShapeType) {
                    case Engine::SHAPE_RECTANGLE:
                        Transformations::generateRectangle(verts, px, py, s, s * 0.75f);
                        break;

                    case Engine::SHAPE_CIRCLE:
                        Transformations::generateCircle(verts, px, py, s / 2);
                        break;

                    case Engine::SHAPE_TRIANGLE:
                        Transformations::generateTriangle(verts, px, py, s);
                        break;

                    case Engine::SHAPE_PENTAGON:
                        Transformations::generateRegularPolygon(verts, px, py, s / 2, 5);
                        break;

                    case Engine::SHAPE_HEXAGON:
                        Transformations::generateRegularPolygon(verts, px, py, s / 2, 6);
                        break;

                    case Engine::SHAPE_STAR:
                        Transformations::generateStar(verts, px, py, s / 2, s * 0.2f);
                        break;
                }

                Shape shape;
                shape.vertices = verts;
                shape.fill[0] = e->fillColor[0];
                shape.fill[1] = e->fillColor[1];
                shape.fill[2] = e->fillColor[2];
                shape.stroke[0] = e->strokeColor[0];
                shape.stroke[1] = e->strokeColor[1];
                shape.stroke[2] = e->strokeColor[2];

                e->shapes.push_back(shape);
            }
            break;
    }
}

// Zoom in/out around world center (0.9x up, 1.1x down)
void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;
    if (ImGui::GetIO().WantCaptureMouse) return;

    float scale = (yoffset > 0) ? 0.9f : 1.1f;

    float cx = (engine->worldMinX + engine->worldMaxX) * 0.5f;
    float cy = (engine->worldMinY + engine->worldMaxY) * 0.5f;

    float hw = (engine->worldMaxX - engine->worldMinX) * 0.5f * scale;
    float hh = (engine->worldMaxY - engine->worldMinY) * 0.5f * scale;

    engine->worldMinX = cx - hw;
    engine->worldMaxX = cx + hw;
    engine->worldMinY = cy - hh;
    engine->worldMaxY = cy + hh;
}

void InputHandler::windowFocusCallback(GLFWwindow* window, int focused) {
    ImGui_ImplGlfw_WindowFocusCallback(window, focused);
}

void InputHandler::cursorEnterCallback(GLFWwindow* window, int entered) {
    ImGui_ImplGlfw_CursorEnterCallback(window, entered);
}

// Handle keyboard: tool hotkeys, Ctrl+Z/Y/S/O, Delete, Esc
void InputHandler::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e || ImGui::GetIO().WantCaptureKeyboard) return;

    if (action == GLFW_PRESS) {
        // Ctrl+Z/Y/S/O shortcuts
        if (key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL)) { e->undo(); return; }
        if (key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL)) { e->redo(); return; }
        if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) {
            if (e->currentFilePath[0]) e->saveShapes(e->currentFilePath);
            else e->saveWithDialog();
            return;
        }
        if (key == GLFW_KEY_O && (mods & GLFW_MOD_CONTROL)) { e->openWithDialog(); return; }
        if (key == GLFW_KEY_C && (mods & GLFW_MOD_CONTROL)) { e->copySelected(); return; }
        if (key == GLFW_KEY_V && (mods & GLFW_MOD_CONTROL)) { e->pasteClipboard(); return; }

        if (key == GLFW_KEY_ENTER || key == GLFW_KEY_KP_ENTER) {
            if (e->isDrawing && e->activeTool == Engine::TOOL_DRAW_LINE) {
                if (e->currentShape.size() >= 2) {
                    e->pushUndo();
                    Shape s;
                    s.vertices = e->currentShape;
                    s.closed = false;
                    s.fill[0] = e->fillColor[0];
                    s.fill[1] = e->fillColor[1];
                    s.fill[2] = e->fillColor[2];
                    s.stroke[0] = e->strokeColor[0];
                    s.stroke[1] = e->strokeColor[1];
                    s.stroke[2] = e->strokeColor[2];
                    e->shapes.push_back(s);
                }
                e->isDrawing = false;
                e->currentShape.clear();
            }
            return;
        }

        if (key == GLFW_KEY_ESCAPE) {
            if (e->isDrawing) {
                e->pushUndo();
                e->isDrawing = false;
                e->currentShape.clear();
            } else {
                glfwSetWindowShouldClose(window, true);
            }
            return;
        }

        // Tool hotkeys without modifier keys
        if (key == GLFW_KEY_1) {
            e->activeTool = Engine::TOOL_SELECT;
            return;
        }

        if (key == GLFW_KEY_2) {
            e->activeTool = Engine::TOOL_DRAW_POLYGON;
            return;
        }

        if (key == GLFW_KEY_3) {
            e->activeTool = Engine::TOOL_DRAW_LINE;
            return;
        }

        if (key == GLFW_KEY_4) {
            e->activeTool = Engine::TOOL_PEN;
            return;
        }

        if (key == GLFW_KEY_5) {
            e->activeTool = Engine::TOOL_FILL;
            return;
        }

        if (key == GLFW_KEY_M) {
            e->activeTool = Engine::TOOL_MOVE;
            return;
        }

        if (key == GLFW_KEY_R && !(mods & GLFW_MOD_CONTROL)) {
            e->activeTool = Engine::TOOL_ROTATE;
            return;
        }

        // O = Rotate around origin
        if (key == GLFW_KEY_O && !(mods & GLFW_MOD_CONTROL)) {
            e->activeTool = Engine::TOOL_ROTATE_ORIGIN;
            return;
        }

        if (key == GLFW_KEY_S && !(mods & GLFW_MOD_CONTROL)) {
            e->activeTool = Engine::TOOL_SCALE;
            return;
        }

        // Delete selected shape(s)
        if (key == GLFW_KEY_DELETE || key == GLFW_KEY_BACKSPACE) {
            if (!e->selectedShapes.empty()) {
                e->pushUndo();
                std::vector<int> sorted = e->selectedShapes;
                std::sort(sorted.begin(), sorted.end());
                for (int i = (int)sorted.size() - 1; i >= 0; i--)
                    e->shapes.erase(e->shapes.begin() + sorted[i]);
                e->clearSelection();
            }

            return;
        }
    }
}