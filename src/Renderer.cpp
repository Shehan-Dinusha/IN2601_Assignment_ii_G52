#include "Renderer.h"
#include "Engine.h"
#include "Clipping.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

Renderer::Renderer(Engine& eng) : engine(eng) {}

void Renderer::render() {
    int canvasX = static_cast<int>(engine.panelWidth);
    int canvasW = engine.windowWidth - canvasX;
    glViewport(canvasX, 0, canvasW, engine.windowHeight);

    glClearColor(0.15f, 0.18f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(
        engine.worldMinX,
        engine.worldMaxX,
        engine.worldMinY,
        engine.worldMaxY,
        -1.0,
        1.0
    );

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawGrid();

    BoundingBox clipBox(engine.worldMinX, engine.worldMaxX, engine.worldMinY, engine.worldMaxY);

    for (auto& shape : engine.shapes) {
        std::vector<Point2D> clipped = shape.vertices;
        Clipping::sutherlandHodgmanClipPolygon(clipped, clipBox);

        if (shape.closed && clipped.size() >= 3) {
            glColor3f(shape.fill[0], shape.fill[1], shape.fill[2]);
            scanLineFill(clipped);
        }

        glColor3f(shape.stroke[0], shape.stroke[1], shape.stroke[2]);
        if (&shape == engine.selectedShape) {
            glColor3f(1.0f, 0.8f, 0.2f);
        }
        glBegin(shape.closed ? GL_LINE_LOOP : GL_LINE_STRIP);
        for (auto& pt : clipped) glVertex2f(pt.x, pt.y);
        glEnd();
    }

    if (engine.isDrawing && !engine.currentShape.empty()) {
        if (engine.activeTool == Engine::TOOL_DRAW_LINE) {
            double mx, my;
            glfwGetCursorPos(engine.window, &mx, &my);
            float wx, wy;
            screenToWorld(mx, my, wx, wy);

            glColor3f(0.3f, 0.8f, 0.3f);
            glBegin(GL_LINE_STRIP);
            for (auto& pt : engine.currentShape) glVertex2f(pt.x, pt.y);
            glVertex2f(wx, wy);
            glEnd();

            glPointSize(6.0f);
            glColor3f(0.5f, 1.0f, 0.5f);
            glBegin(GL_POINTS);
            for (auto& pt : engine.currentShape) glVertex2f(pt.x, pt.y);
            glEnd();
        } else {
            glColor3f(0.5f, 1.0f, 0.5f);
            glBegin(GL_LINE_STRIP);
            for (auto& pt : engine.currentShape) glVertex2f(pt.x, pt.y);
            if (engine.currentShape.size() >= 3)
                glVertex2f(engine.currentShape[0].x, engine.currentShape[0].y);
            glEnd();

            glPointSize(6.0f);
            glBegin(GL_POINTS);
            for (auto& pt : engine.currentShape) glVertex2f(pt.x, pt.y);
            glEnd();
        }
    }

    if (engine.isSelecting) {
        float minX = std::min(engine.selBoxX1, engine.selBoxX2);
        float maxX = std::max(engine.selBoxX1, engine.selBoxX2);
        float minY = std::min(engine.selBoxY1, engine.selBoxY2);
        float maxY = std::max(engine.selBoxY1, engine.selBoxY2);

        glColor4f(0.3f, 0.6f, 1.0f, 0.15f);
        glBegin(GL_QUADS);
        glVertex2f(minX, minY);
        glVertex2f(maxX, minY);
        glVertex2f(maxX, maxY);
        glVertex2f(minX, maxY);
        glEnd();

        glColor3f(0.3f, 0.6f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(minX, minY);
        glVertex2f(maxX, minY);
        glVertex2f(maxX, maxY);
        glVertex2f(minX, maxY);
        glEnd();
    }
}

void Renderer::drawGrid() {
    if (!engine.showGrid) return;

    glColor3f(0.22f, 0.26f, 0.29f);
    float step = 10.0f;

    float x0 = std::floor(engine.worldMinX / step) * step;
    float y0 = std::floor(engine.worldMinY / step) * step;

    glBegin(GL_LINES);

    for (float x = x0; x <= engine.worldMaxX; x += step) {
        glVertex2f(x, engine.worldMinY);
        glVertex2f(x, engine.worldMaxY);
    }

    for (float y = y0; y <= engine.worldMaxY; y += step) {
        glVertex2f(engine.worldMinX, y);
        glVertex2f(engine.worldMaxX, y);
    }

    glEnd();
}

void Renderer::scanLineFill(const std::vector<Point2D>& polygon) {
    if (polygon.size() < 3) return;

    float yMin = polygon[0].y, yMax = polygon[0].y;
    for (auto& pt : polygon) {
        if (pt.y < yMin) yMin = pt.y;
        if (pt.y > yMax) yMax = pt.y;
    }

    int iyMin = static_cast<int>(std::ceil(yMin));
    int iyMax = static_cast<int>(std::floor(yMax));
    size_t n = polygon.size();

    glBegin(GL_LINES);
    for (int y = iyMin; y <= iyMax; ++y) {
        float scanY = static_cast<float>(y);
        std::vector<float> xs;

        for (size_t i = 0; i < n; ++i) {
            float xi = polygon[i].x, yi = polygon[i].y;
            float xj = polygon[(i + 1) % n].x, yj = polygon[(i + 1) % n].y;
            if ((yi > scanY) != (yj > scanY)) {
                xs.push_back(xi + (scanY - yi) * (xj - xi) / (yj - yi));
            }
        }

        if (xs.size() < 2) continue;
        std::sort(xs.begin(), xs.end());
        for (size_t i = 0; i + 1 < xs.size(); i += 2) {
            glVertex2f(xs[i], scanY);
            glVertex2f(xs[i + 1], scanY);
        }
    }
    glEnd();
}

void Renderer::screenToWorld(double screenX, double screenY, float& worldX, float& worldY) {
    float canvasX = static_cast<float>(screenX) - engine.panelWidth;
    float canvasW = engine.windowWidth - engine.panelWidth;
    worldX = engine.worldMinX + (canvasX / canvasW) * (engine.worldMaxX - engine.worldMinX);
    worldY = engine.worldMaxY - (static_cast<float>(screenY) / engine.windowHeight) * (engine.worldMaxY - engine.worldMinY);
}