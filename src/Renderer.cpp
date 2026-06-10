#include "Renderer.h"
#include "Engine.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>

Renderer::Renderer(Engine& eng) : engine(eng) {}

// TEMPORARY SIMPLE RENDERER
// This is only to display shapes and test Member 2 transformations.
void Renderer::render() {
    glClearColor(0.15f, 0.18f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

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

    // Draw all saved shapes
    for (const auto& shape : engine.shapes) {
        bool isSelected = (&shape == engine.selectedShape);

        // Fill closed shapes
        if (shape.closed && shape.vertices.size() >= 3) {
            if (isSelected) {
                glColor3f(1.0f, 0.8f, 0.2f);   // selected = gold
            } else {
                glColor3f(shape.fill[0], shape.fill[1], shape.fill[2]);
            }

            glBegin(GL_POLYGON);
            for (const auto& pt : shape.vertices) {
                glVertex2f(pt.x, pt.y);
            }
            glEnd();
        }

        // Draw outline
        if (isSelected) {
            glColor3f(1.0f, 0.8f, 0.2f);
            glLineWidth(3.0f);
        } else {
            glColor3f(shape.stroke[0], shape.stroke[1], shape.stroke[2]);
            glLineWidth(2.0f);
        }

        if (shape.closed) {
            glBegin(GL_LINE_LOOP);
        } else {
            glBegin(GL_LINE_STRIP);
        }

        for (const auto& pt : shape.vertices) {
            glVertex2f(pt.x, pt.y);
        }

        glEnd();
        glLineWidth(1.0f);
    }

    // Draw in-progress shape preview
    if (!engine.currentShape.empty()) {
        glColor3f(engine.strokeColor[0], engine.strokeColor[1], engine.strokeColor[2]);
        glLineWidth(2.0f);

        glBegin(GL_LINE_STRIP);
        for (const auto& pt : engine.currentShape) {
            glVertex2f(pt.x, pt.y);
        }
        glEnd();

        glPointSize(5.0f);
        glBegin(GL_POINTS);
        for (const auto& pt : engine.currentShape) {
            glVertex2f(pt.x, pt.y);
        }
        glEnd();

        glLineWidth(1.0f);
        glPointSize(1.0f);
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

// Temporary fill function.
// Final scan-line fill is Member 4's responsibility.
void Renderer::scanLineFill(const std::vector<Point2D>& polygon) {
    if (polygon.size() < 3) return;

    glBegin(GL_POLYGON);
    for (const auto& pt : polygon) {
        glVertex2f(pt.x, pt.y);
    }
    glEnd();
}

void Renderer::screenToWorld(double screenX, double screenY, float& worldX, float& worldY) {
    worldX = engine.worldMinX + (screenX / engine.windowWidth) * (engine.worldMaxX - engine.worldMinX);
    worldY = engine.worldMaxY - (screenY / engine.windowHeight) * (engine.worldMaxY - engine.worldMinY);
}