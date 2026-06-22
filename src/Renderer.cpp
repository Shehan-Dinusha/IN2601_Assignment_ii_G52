#include "Renderer.h"
#include "Engine.h"
#include "Clipping.h"
#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>

#ifndef CALLBACK
#define CALLBACK __stdcall
#endif
#include <GL/glu.h>

static void CALLBACK tessBegin(GLenum type) { glBegin(type); }
static void CALLBACK tessEnd() { glEnd(); }
static void CALLBACK tessVertex(void* v) {
    glVertex2dv(static_cast<GLdouble*>(v));
}
static void CALLBACK tessCombine(GLdouble coords[3], void**, GLfloat*, void** outData) {
    GLdouble* vert = new GLdouble[3];
    vert[0] = coords[0]; vert[1] = coords[1]; vert[2] = coords[2];
    *outData = vert;
}
static void CALLBACK tessError(GLenum) {}

Renderer::Renderer(Engine& eng) : engine(eng) {}

void Renderer::render() {
    int canvasX = static_cast<int>(engine.panelWidth);
    int canvasW = engine.windowWidth - canvasX;
    glViewport(canvasX, 0, canvasW, engine.windowHeight);

    glClearColor(0.15f, 0.18f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float aspect = canvasW / (float)engine.windowHeight;
    float cx = (engine.worldMinX + engine.worldMaxX) * 0.5f;
    float cy = (engine.worldMinY + engine.worldMaxY) * 0.5f;
    float halfH = (engine.worldMaxY - engine.worldMinY) * 0.5f;
    float halfW = halfH * aspect;
    float effLeft = cx - halfW, effRight = cx + halfW;
    float effBottom = cy - halfH, effTop = cy + halfH;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(effLeft, effRight, effBottom, effTop, -1.0, 1.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    drawGrid(effLeft, effRight);

    BoundingBox clipBox(effLeft, effRight, effBottom, effTop);

    for (int idx = 0; idx < (int)engine.shapes.size(); idx++) {
        auto& shape = engine.shapes[idx];

        if (shape.closed) {
            std::vector<Point2D> clipped = shape.vertices;
            Clipping::sutherlandHodgmanClipPolygon(clipped, clipBox);

            if (clipped.size() >= 3) {
                glColor3f(shape.fill[0], shape.fill[1], shape.fill[2]);
                floodFill(clipped);
            }

            glColor3f(shape.stroke[0], shape.stroke[1], shape.stroke[2]);
            if (engine.isSelected(idx)) glColor3f(1.0f, 0.8f, 0.2f);
            glBegin(GL_LINE_LOOP);
            for (auto& pt : clipped) glVertex2f(pt.x, pt.y);
            glEnd();
        } else {
            glColor3f(shape.stroke[0], shape.stroke[1], shape.stroke[2]);
            if (engine.isSelected(idx)) glColor3f(1.0f, 0.8f, 0.2f);
            glBegin(GL_LINES);
            for (size_t si = 0; si + 1 < shape.vertices.size(); si++) {
                Point2D p1 = shape.vertices[si];
                Point2D p2 = shape.vertices[si + 1];
                if (Clipping::liangBarskyClipLine(p1, p2, clipBox))
                {
                    glVertex2f(p1.x, p1.y);
                    glVertex2f(p2.x, p2.y);
                }
            }
            glEnd();
        }
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

void Renderer::drawGrid(float effLeft, float effRight) {
    if (!engine.showGrid) return;

    glColor3f(0.22f, 0.26f, 0.29f);
    float step = 10.0f;

    float x0 = std::floor(effLeft / step) * step;
    float y0 = std::floor(engine.worldMinY / step) * step;

    glBegin(GL_LINES);

    for (float x = x0; x <= effRight; x += step) {
        glVertex2f(x, engine.worldMinY);
        glVertex2f(x, engine.worldMaxY);
    }

    for (float y = y0; y <= engine.worldMaxY; y += step) {
        glVertex2f(effLeft, y);
        glVertex2f(effRight, y);
    }

    glEnd();
}

void Renderer::floodFill(const std::vector<Point2D>& polygon) {
    if (polygon.size() < 3) return;

    std::vector<GLdouble> verts(polygon.size() * 3);
    for (size_t i = 0; i < polygon.size(); ++i) {
        verts[i * 3 + 0] = polygon[i].x;
        verts[i * 3 + 1] = polygon[i].y;
        verts[i * 3 + 2] = 0.0;
    }

    GLUtesselator* tess = gluNewTess();
    if (!tess) return;

    gluTessCallback(tess, GLU_TESS_BEGIN,   (void(CALLBACK*)())tessBegin);
    gluTessCallback(tess, GLU_TESS_END,     (void(CALLBACK*)())tessEnd);
    gluTessCallback(tess, GLU_TESS_VERTEX,  (void(CALLBACK*)())tessVertex);
    gluTessCallback(tess, GLU_TESS_COMBINE, (void(CALLBACK*)())tessCombine);
    gluTessCallback(tess, GLU_TESS_ERROR,   (void(CALLBACK*)())tessError);

    gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);

    gluTessBeginPolygon(tess, nullptr);
    gluTessBeginContour(tess);
    for (size_t i = 0; i < polygon.size(); ++i)
        gluTessVertex(tess, &verts[i * 3], &verts[i * 3]);
    gluTessEndContour(tess);
    gluTessEndPolygon(tess);

    gluDeleteTess(tess);
}

void Renderer::screenToWorld(double screenX, double screenY, float& worldX, float& worldY) {
    float canvasX = static_cast<float>(screenX) - engine.panelWidth;
    float canvasW = engine.windowWidth - engine.panelWidth;
    float aspect = canvasW / (float)engine.windowHeight;
    float cx = (engine.worldMinX + engine.worldMaxX) * 0.5f;
    float cy = (engine.worldMinY + engine.worldMaxY) * 0.5f;
    float halfH = (engine.worldMaxY - engine.worldMinY) * 0.5f;
    float halfW = halfH * aspect;
    float left = cx - halfW, right = cx + halfW;
    float top = cy + halfH, bottom = cy - halfH;
    worldX = left + (canvasX / canvasW) * (right - left);
    worldY = top - (static_cast<float>(screenY) / engine.windowHeight) * (top - bottom);
}