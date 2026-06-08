#include "Renderer.h"
#include "Engine.h"
#include "Clipping.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cmath>

Renderer::Renderer(Engine& eng) : engine(eng) {}

// ============================================================
// MEMBER 4 TODO: Implement render()
// 1. Clear background, set up orthographic projection
// 2. Draw grid (drawGrid())
// 3. For each shape: clip to viewport (sutherlandHodgmanClipPolygon),
//    fill with scanLineFill, draw stroke outline
// 4. Draw in-progress shape preview if isDrawing
// 5. Selected shape renders in gold (1.0, 0.8, 0.2)
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void Renderer::render() {
    glClearColor(0.15f, 0.18f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(engine.worldMinX, engine.worldMaxX, engine.worldMinY, engine.worldMaxY, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawGrid();

    // TODO: render shapes from engine.shapes
    // TODO: render in-progress drawing preview
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

// ============================================================
// MEMBER 4 TODO: Implement scanLineFill
// 1. Find yMin/yMax of polygon
// 2. For each integer scan line: compute edge x-intersections,
//    sort, draw horizontal segments between even-odd pairs
// Use glBegin(GL_LINES) / glEnd()
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void Renderer::scanLineFill(const std::vector<Point2D>& polygon) {
    (void)polygon;
    // TODO
}

void Renderer::screenToWorld(double screenX, double screenY, float& worldX, float& worldY) {
    worldX = engine.worldMinX + (screenX / engine.windowWidth) * (engine.worldMaxX - engine.worldMinX);
    worldY = engine.worldMaxY - (screenY / engine.windowHeight) * (engine.worldMaxY - engine.worldMinY);
}
