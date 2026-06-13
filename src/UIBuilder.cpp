#include "UIBuilder.h"
#include "Engine.h"
#include "Transformations.h"
#include "Clipping.h"
#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <algorithm>

UIBuilder::UIBuilder(Engine& eng) : engine(eng) {}

void UIBuilder::buildUI() {
    float menuHeight = 0.0f;

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S")) engine.saveWithDialog();
            if (ImGui::MenuItem("Open", "Ctrl+O")) engine.openWithDialog();
            if (ImGui::MenuItem("Export SVG")) engine.exportWithDialog();
            ImGui::Separator();
            if (ImGui::MenuItem("Clear All")) {
                engine.shapes.clear();
                engine.currentShape.clear();
                engine.clearSelection();
                engine.isDrawing = false;
            }
            ImGui::Separator();
            if (ImGui::MenuItem("Exit", "Esc")) glfwSetWindowShouldClose(engine.window, true);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) engine.undo();
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) engine.redo();
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Grid", NULL, &engine.showGrid);
            ImGui::MenuItem("Snap to Grid", NULL, &engine.snapToGridEnabled);
            if (ImGui::MenuItem("Reset Zoom")) engine.setWorldWindow(-100.0f, 100.0f, -100.0f, 100.0f);
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Help")) {
            ImGui::Text("Shortcuts:");
            ImGui::Text("  Ctrl+Z=Undo  Ctrl+Y=Redo  Ctrl+S=Save  Ctrl+O=Open");
            ImGui::Text("  Ctrl+C=Copy  Ctrl+V=Paste  (10px offset)");
            ImGui::Text("  1=Select  2=Polygon  3=Line  4=Pen  5=Fill");
            ImGui::Text("  M=Move  R=Rotate  O=Rot Origin  S=Scale");
            ImGui::Text("  Del=Delete  Enter=Finish Line  Esc=Cancel");
            ImGui::Separator();
            ImGui::Text("Select: Click or drag box to select");
            ImGui::Text("Polygon: Left-click = vertex (max 100), Right-click = finish");
            ImGui::Text("Line: Left-click = vertex, Right-click = undo last, Enter = finish");
            ImGui::Text("Pen: Click-drag freehand");
            ImGui::Text("Fill: Click shape to recolor; click empty area = fill enclosed region");
            ImGui::Text("Move/Rotate/Scale: Select shape, then click-drag");
            ImGui::Text("Z-order: Bring to Front / Send to Back in Selection panel");
            ImGui::Separator();
            ImGui::Text("Grid: View > Show Grid  |  Snap to Grid: View > Snap to Grid");
            ImGui::Text("Snap: Vertices auto-snap to existing geometry within 5 units");
            ImGui::Text("Export: File > Export SVG");
            ImGui::Text("Undo depth: 100 levels");
            ImGui::EndMenu();
        }

        menuHeight = ImGui::GetWindowHeight();
        ImGui::EndMainMenuBar();
    }

    ImGui::SetNextWindowPos(ImVec2(0, menuHeight));
    ImGui::SetNextWindowSize(ImVec2(engine.panelWidth, engine.windowHeight - menuHeight));
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

    ImGui::BeginDisabled(engine.selectedShapes.empty());
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

    float* fillPtr = engine.firstSelected() >= 0 ? engine.shapes[engine.firstSelected()].fill : engine.fillColor;
    float* strokePtr = engine.firstSelected() >= 0 ? engine.shapes[engine.firstSelected()].stroke : engine.strokeColor;

    if (ImGui::ColorEdit3("Fill", fillPtr) && engine.firstSelected() >= 0)
        for (int idx : engine.selectedShapes)
            for (int c = 0; c < 3; c++) engine.shapes[idx].fill[c] = fillPtr[c];
    if (ImGui::ColorEdit3("Stroke", strokePtr) && engine.firstSelected() >= 0)
        for (int idx : engine.selectedShapes)
            for (int c = 0; c < 3; c++) engine.shapes[idx].stroke[c] = strokePtr[c];

    ImGui::Spacing();
    ImGui::Separator();

    if (!engine.selectedShapes.empty()) {
        ImGui::TextUnformatted("Selection");
        ImGui::Text("%zu shapes selected", engine.selectedShapes.size());

        if (ImGui::Button("Mirror H", ImVec2(-1, 26))) {
            for (int idx : engine.selectedShapes) engine.shapes[idx].mirror(true, false);
        }

        if (ImGui::Button("Mirror V", ImVec2(-1, 26))) {
            for (int idx : engine.selectedShapes) engine.shapes[idx].mirror(false, true);
        }

        if (ImGui::Button("Bring to Front", ImVec2(-1, 26))) {
            engine.bringToFront();
        }

        if (ImGui::Button("Send to Back", ImVec2(-1, 26))) {
            engine.sendToBack();
        }

        if (ImGui::Button("Delete", ImVec2(-1, 26))) {
            engine.pushUndo();
            std::vector<int> sorted = engine.selectedShapes;
            std::sort(sorted.begin(), sorted.end());
            for (int i = (int)sorted.size() - 1; i >= 0; i--)
                engine.shapes.erase(engine.shapes.begin() + sorted[i]);
            engine.clearSelection();
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

    ImGui::SetNextWindowPos(ImVec2(engine.panelWidth, engine.windowHeight - 32));
    ImGui::SetNextWindowSize(ImVec2(engine.windowWidth - engine.panelWidth, 32));

    ImGui::Begin("StatusBar", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar);

    float zoomPct = 200.0f / (engine.worldMaxX - engine.worldMinX) * 100.0f;

    if (engine.activeTool == Engine::TOOL_SCALE && engine.isDragging && !engine.selectedShapes.empty()) {
        ImGui::Text("  Tool: %s  |  (%.1f, %.1f)  |  Shapes: %zu  |  Zoom: %.0f%%  |  Scale: %.2fx",
            label, wx, wy, engine.shapes.size(), zoomPct, engine.dragScaleFactor);
    } else if ((engine.activeTool == Engine::TOOL_ROTATE ||
                engine.activeTool == Engine::TOOL_ROTATE_ORIGIN) &&
                engine.isDragging && !engine.selectedShapes.empty()) {
        ImGui::Text("  Tool: %s  |  (%.1f, %.1f)  |  Shapes: %zu  |  Zoom: %.0f%%  |  Angle: %.1f",
            label, wx, wy, engine.shapes.size(), zoomPct, engine.dragAngle);
    } else {
        ImGui::Text("  Tool: %s  |  (%.1f, %.1f)  |  Shapes: %zu  |  Zoom: %.0f%%",
            label, wx, wy, engine.shapes.size(), zoomPct);
    }

    ImGui::End();

    // Overlay readout during rotate/scale drag
    if (engine.isDragging && !engine.selectedShapes.empty()) {
        ImGui::SetNextWindowPos(ImVec2((float)mx + 15, (float)my - 30));
        ImGui::SetNextWindowBgAlpha(0.7f);
        ImGui::Begin("##dragOverlay", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoInputs);
        if (engine.activeTool == Engine::TOOL_SCALE)
            ImGui::Text("Scale: %.2fx", engine.dragScaleFactor);
        else if (engine.activeTool == Engine::TOOL_ROTATE || engine.activeTool == Engine::TOOL_ROTATE_ORIGIN)
            ImGui::Text("Angle: %.1f", engine.dragAngle);
        else if (engine.activeTool == Engine::TOOL_MOVE)
            ImGui::Text("Move");
        ImGui::End();
    }
}