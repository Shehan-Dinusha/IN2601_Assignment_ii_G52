#ifndef RENDERER_H
#define RENDERER_H

#include <vector>
#include "Shape.h"

class Engine;

class Renderer {
public:
    explicit Renderer(Engine& engine);

    void render();
    void drawGrid();
    void scanLineFill(const std::vector<Point2D>& polygon);
    void screenToWorld(double screenX, double screenY, float& worldX, float& worldY);

private:
    Engine& engine;
};

#endif
