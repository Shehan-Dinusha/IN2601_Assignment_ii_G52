#include "UIBuilder.h"
#include "Engine.h"
#include "Transformations.h"
#include "Clipping.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

UIBuilder::UIBuilder(Engine& eng) : engine(eng) {}

void UIBuilder::buildUI() {
    float menuHeight = 0.0f;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Clear All")) {
                engine.shapes.clear();
                engine.currentShape.clear();
                engine.selectedShape = nullptr;
                engine.isDrawing = false;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc")) glfwSetWindowShouldClose(engine.window, true);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Grid", NULL, &engine.showGrid);
            if (ImGui::MenuItem("Reset Zoom")) engine.setWorldWindow(-100.0f, 100.0f, -100.0f, 100.0f);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::Text("Select: Click to select a shape");
            ImGui::Text("Polygon: Left-click = vertex, Right-click = finish");
            ImGui::Text("Line: Left-click = vertex, Right-click = undo last point, Esc = finish");
            ImGui::Text("Pen: Click-drag to draw freehand");
            ImGui::Text("Fill: Click an object to fill it (fill + stroke color)");
            ImGui::Text("Move/Rotate/Scale: Select, then click-drag");
            ImGui::Text("Rotate: Drag left/right to spin around shape center");
            ImGui::Text("Rotate Origin: Drag left/right to rotate around canvas origin (0,0)");
            ImGui::Text("Pan: Middle-drag  |  Zoom: Scroll");
            ImGui::EndMenu();
        }

        menuHeight = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    float panelW = 195.0f;
    ImGui::SetNextWindowPos(ImVec2(0, menuHeight));
    ImGui::SetNextWindowSize(ImVec2(panelW, engine.windowHeight - menuHeight));
    ImGui::Begin("Tools", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

    auto activeBtn = [&](Engine::Tool t) -> bool {
        if (engine.activeTool == t) {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.25f, 0.50f, 0.85f, 1.00f));
            return true;
        }
        return false;
    };

    auto endActive = [&](bool pushed) {
        if (pushed) ImGui::PopStyleColor();
    };

    auto clearDraw = [&]() {
        engine.currentShape.clear();
        engine.isDrawing = false;
    };

    auto toolBtn = [&](const char* label, Engine::Tool t) {
        bool pushed = activeBtn(t);
        if (ImGui::Button(label, ImVec2(-1, 28))) {
            clearDraw();
            engine.activeTool = t;
        }
        endActive(pushed);
    };

    ImGui::TextUnformatted("Draw");
    ImGui::Separator();
    toolBtn("Select",   Engine::TOOL_SELECT);
    toolBtn("Polygon",  Engine::TOOL_DRAW_POLYGON);
    toolBtn("Line",     Engine::TOOL_DRAW_LINE);
    toolBtn("Pen",      Engine::TOOL_PEN);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Edit");
    ImGui::Separator();
    toolBtn("Fill",     Engine::TOOL_FILL);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Transform");
    ImGui::Separator();

    ImGui::BeginDisabled(!engine.selectedShape);
    toolBtn("Move",          Engine::TOOL_MOVE);
    toolBtn("Rotate",        Engine::TOOL_ROTATE);
    toolBtn("Rotate Origin", Engine::TOOL_ROTATE_ORIGIN);
    toolBtn("Scale",         Engine::TOOL_SCALE);
    ImGui::EndDisabled();

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Shapes");
    ImGui::Separator();

    const char* shapeItems[] = { "Rectangle", "Circle", "Triangle", "Pentagon", "Hexagon", "Star" };
    ImGui::Combo("##shapetype", (int*)&engine.activeShapeType, shapeItems, IM_ARRAYSIZE(shapeItems));
    ImGui::SliderFloat("Size", &engine.shapeSize, 5.0f, 100.0f, "%.0f");
    toolBtn("Place", Engine::TOOL_SHAPE);

    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Colors");

    float* fillPtr = engine.selectedShape ? engine.selectedShape->fill : engine.fillColor;
    float* strokePtr = engine.selectedShape ? engine.selectedShape->stroke : engine.strokeColor;

    ImGui::ColorEdit3("Fill", fillPtr);
    ImGui::ColorEdit3("Stroke", strokePtr);

    ImGui::Spacing();
    ImGui::Separator();

    if (engine.selectedShape) {
        ImGui::TextUnformatted("Selection");
        ImGui::Text("Vertices: %zu", engine.selectedShape->vertices.size());

        if (ImGui::Button("Mirror H", ImVec2(-1, 26))) {
            engine.selectedShape->mirror(true, false);
        }

        if (ImGui::Button("Mirror V", ImVec2(-1, 26))) {
            engine.selectedShape->mirror(false, true);
        }

        if (ImGui::Button("Delete", ImVec2(-1, 26))) {
            for (auto it = engine.shapes.begin(); it != engine.shapes.end(); ++it) {
                if (&*it == engine.selectedShape) {
                    engine.shapes.erase(it);
                    break;
                }
            }
            engine.selectedShape = nullptr;
        }
    } else {
        ImGui::TextDisabled("No selection");
    }

    ImGui::End();

    // Status bar
    double mx, my;
    glfwGetCursorPos(engine.window, &mx, &my);

    float wx, wy;
    engine.renderer.screenToWorld(mx, my, wx, wy);

 const char* labels[] = { "Select", "Polygon", "Line", "Pen", "Fill", "Move", "Rotate", "Rotate Origin", "Scale", "Shapes" };

    const int labelCount = IM_ARRAYSIZE(labels);
    const char* label = (engine.activeTool >= 0 && engine.activeTool < labelCount)
        ? labels[engine.activeTool]
        : "?";

    ImGui::SetNextWindowPos(ImVec2(panelW, engine.windowHeight - 32));
    ImGui::SetNextWindowSize(ImVec2(engine.windowWidth - panelW, 32));

    ImGui::Begin("StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);

    if (engine.activeTool == Engine::TOOL_SCALE && engine.isDragging && engine.selectedShape) {
        ImGui::Text("  Tool: %s  |  (%.1f, %.1f)  |  Shapes: %zu  |  Scale: %.2fx",
            label, wx, wy, engine.shapes.size(), engine.dragScaleFactor);
    } else if ((engine.activeTool == Engine::TOOL_ROTATE ||
                engine.activeTool == Engine::TOOL_ROTATE_ORIGIN) &&
                engine.isDragging && engine.selectedShape) {
        ImGui::Text("  Tool: %s  |  (%.1f, %.1f)  |  Shapes: %zu  |  Angle: %.1f",
            label, wx, wy, engine.shapes.size(), engine.dragAngle);
    } else {
        ImGui::Text("  Tool: %s  |  (%.1f, %.1f)  |  Shapes: %zu",
            label, wx, wy, engine.shapes.size());
    }

    ImGui::End();
}