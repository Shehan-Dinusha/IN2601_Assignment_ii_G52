# IN2601 Assignment II — Complete Project Reference

This file documents every task function's **final implementation** from the completed project.  
Each member's AI agent can use this as a reference. All function signatures, algorithm notes, and verification steps are included.

**Build order:** Member 1 → Member 2 → Members 3,4,5 (parallel after 1 & 2).

---

## Member 1 — Core Math, Matrix Algebra & Polygon Clipping

**Files:** `src/Transformations.cpp`, `src/Clipping.cpp`

---

### `Matrix3x3::operator*(const Matrix3x3& other) const`

```cpp
// File: Transformations.cpp
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 3; ++k)
                result.m[i][j] += m[i][k] * other.m[k][j];
        }
    return result;
}
```

Standard 3×3 matrix multiplication. `result[i][j] = sum over k of m[i][k] * other.m[k][j]`.

---

### `Matrix3x3::operator*(const Point2D& point) const`

```cpp
// File: Transformations.cpp
Point2D Matrix3x3::operator*(const Point2D& point) const {
    Point2D result;
    result.x = m[0][0] * point.x + m[0][1] * point.y + m[0][2] * point.w;
    result.y = m[1][0] * point.x + m[1][1] * point.y + m[1][2] * point.w;
    result.w = m[2][0] * point.x + m[2][1] * point.y + m[2][2] * point.w;
    return result;
}
```

Multiply a 3×3 matrix by a homogeneous 2D column vector `(x, y, w)`. Standard matrix-vector multiplication.

---

### `Transformations::getTranslationMatrix(float tx, float ty)`

```cpp
// File: Transformations.cpp
Matrix3x3 Transformations::getTranslationMatrix(float tx, float ty) {
    Matrix3x3 mat;
    mat.m[0][2] = tx;
    mat.m[1][2] = ty;
    return mat;
}
```

Returns identity matrix with translation components in column 2:
```
[1  0  tx]
[0  1  ty]
[0  0  1 ]
```

---

### `Transformations::getScalingMatrix(float sx, float sy)`

```cpp
// File: Transformations.cpp
Matrix3x3 Transformations::getScalingMatrix(float sx, float sy) {
    Matrix3x3 mat;
    mat.m[0][0] = sx;
    mat.m[1][1] = sy;
    return mat;
}
```

Returns:
```
[sx 0  0]
[0  sy 0]
[0  0  1]
```

---

### `Transformations::getRotationMatrix(float angleRadians)`

```cpp
// File: Transformations.cpp
Matrix3x3 Transformations::getRotationMatrix(float angleRadians) {
    Matrix3x3 mat;
    float c = std::cos(angleRadians), s = std::sin(angleRadians);
    mat.m[0][0] = c; mat.m[0][1] = -s;
    mat.m[1][0] = s; mat.m[1][1] =  c;
    return mat;
}
```

Returns counter-clockwise rotation matrix:
```
[cos -sin  0]
[sin  cos  0]
[0    0    1]
```

Uses `std::cos` / `std::sin` from `<cmath>`.

---

### `Clipping::sutherlandHodgmanClipPolygon(std::vector<Point2D>& polygon, const BoundingBox& box)`

```cpp
// File: Clipping.cpp — includes helper (add as static function in Clipping.cpp)
static void clipAgainstEdge(
    const std::vector<Point2D>& poly,
    std::vector<Point2D>& out,
    const BoundingBox& box,
    int edge
) {
    out.clear();
    if (poly.empty()) return;
    size_t n = poly.size();

    auto isInside = [&](const Point2D& p) -> bool {
        switch (edge) {
            case 0: return p.x >= box.xMin;  // left
            case 1: return p.x <= box.xMax;  // right
            case 2: return p.y >= box.yMin;  // bottom
            case 3: return p.y <= box.yMax;  // top
            default: return false;
        }
    };

    auto intersect = [&](const Point2D& a, const Point2D& b) -> Point2D {
        Point2D p;
        if (edge == 0 || edge == 1) {
            float x = (edge == 0) ? box.xMin : box.xMax;
            p.y = a.y + (b.y - a.y) * (x - a.x) / (b.x - a.x);
            p.x = x;
        } else {
            float y = (edge == 2) ? box.yMin : box.yMax;
            p.x = a.x + (b.x - a.x) * (y - a.y) / (b.y - a.y);
            p.y = y;
        }
        p.w = 1.0f;
        return p;
    };

    for (size_t i = 0; i < n; ++i) {
        const Point2D& cur = poly[i];
        const Point2D& nxt = poly[(i + 1) % n];

        bool curInside = isInside(cur);
        bool nxtInside = isInside(nxt);

        if (curInside) {
            out.push_back(cur);
            if (!nxtInside)
                out.push_back(intersect(cur, nxt));
        } else if (nxtInside) {
            out.push_back(intersect(cur, nxt));
            out.push_back(nxt);
        }
    }
}

// Main function
bool Clipping::sutherlandHodgmanClipPolygon(std::vector<Point2D>& polygon, const BoundingBox& box) {
    if (polygon.size() < 3) return false;

    std::vector<Point2D> output = polygon;

    for (int edge = 0; edge < 4; ++edge) {
        std::vector<Point2D> input = output;
        clipAgainstEdge(input, output, box, edge);
        if (output.size() < 3) return false;
    }

    polygon = output;
    return true;
}
```

Clips polygon against 4 edges sequentially: left → right → bottom → top. Inside check per edge:
- Edge 0 (left):   `p.x >= box.xMin`
- Edge 1 (right):  `p.x <= box.xMax`
- Edge 2 (bottom): `p.y >= box.yMin`
- Edge 3 (top):    `p.y <= box.yMax`

**Verification:** Draw a shape extending beyond viewport, zoom out — shape should clip at edges.

---

## Member 2 — Applied Geometry & Shape Manipulations

**File:** `src/Transformations.cpp`

---

### `Transformations::translateShape(std::vector<Point2D>& shape, float tx, float ty)`

```cpp
// File: Transformations.cpp
void Transformations::translateShape(std::vector<Point2D>& shape, float tx, float ty) {
    Matrix3x3 t = getTranslationMatrix(tx, ty);
    for (auto& pt : shape) pt = t * pt;
}
```

Applies translation matrix to every vertex. Uses `getTranslationMatrix()` (Member 1) and `operator*` (Member 1).

---

### `Transformations::scaleShape(std::vector<Point2D>& shape, float sx, float sy)`

```cpp
// File: Transformations.cpp
void Transformations::scaleShape(std::vector<Point2D>& shape, float sx, float sy) {
    Matrix3x3 s = getScalingMatrix(sx, sy);
    for (auto& pt : shape) pt = s * pt;
}
```

Applies scaling matrix to every vertex.

---

### `Transformations::rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians)`

```cpp
// File: Transformations.cpp
void Transformations::rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians) {
    Matrix3x3 r = getRotationMatrix(angleRadians);
    for (auto& pt : shape) pt = r * pt;
}
```

Rotates all vertices around (0,0).

---

### `Transformations::rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians)`

```cpp
// File: Transformations.cpp
void Transformations::rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians) {
    Matrix3x3 t1 = getTranslationMatrix(-pivot.x, -pivot.y);
    Matrix3x3 r = getRotationMatrix(angleRadians);
    Matrix3x3 t2 = getTranslationMatrix(pivot.x, pivot.y);
    Matrix3x3 comp = t2 * r * t1;
    for (auto& pt : shape) pt = comp * pt;
}
```

Homogeneous matrix composition: Translate to origin → Rotate → Translate back. This demonstrates why the matrix chain `t2 * r * t1` is applied to each point.

**Verification for all Member 2:** Run the program, select a shape, use Move/Rotate/Scale tools with click-drag on canvas.

---

## Member 3 — Engine Input, Camera & Hit-Testing Algorithm

**Files:** `src/Engine.cpp`, `src/Clipping.cpp`, `src/InputHandler.cpp`

---

### `InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)`

```cpp
// File: InputHandler.cpp
void InputHandler::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos) {
    ImGui_ImplGlfw_CursorPosCallback(window, xpos, ypos);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e) return;

    if (e->isPanning) {
        double dx = (e->lastMouseX - xpos) / e->windowWidth;
        double dy = (ypos - e->lastMouseY) / e->windowHeight;
        float rx = e->worldMaxX - e->worldMinX;
        float ry = e->worldMaxY - e->worldMinY;
        e->worldMinX += dx * rx; e->worldMaxX += dx * rx;
        e->worldMinY += dy * ry; e->worldMaxY += dy * ry;
        e->lastMouseX = xpos; e->lastMouseY = ypos;
        return;
    }

    if (e->isDragging && e->selectedShape) {
        float wx, wy;
        e->renderer.screenToWorld(xpos, ypos, wx, wy);

        if (e->activeTool == Engine::TOOL_MOVE) {
            float dx = wx - e->dragStartX;
            float dy = wy - e->dragStartY;
            e->selectedShape->translate(dx, dy);
            e->dragStartX = wx;
            e->dragStartY = wy;
        } else if (e->activeTool == Engine::TOOL_ROTATE) {
            float angle = static_cast<float>(xpos - e->lastMouseX) * 0.02f;
            Point2D c = e->selectedShape->centroid();
            e->selectedShape->rotate(angle, c);
            e->dragAngle += angle * 180.0f / 3.14159265f;
            if (e->dragAngle < 0.0f) e->dragAngle += 360.0f;
            if (e->dragAngle >= 360.0f) e->dragAngle -= 360.0f;
        } else if (e->activeTool == Engine::TOOL_SCALE) {
            float s = 1.0f + static_cast<float>(xpos - e->lastMouseX) * 0.01f;
            Point2D c = e->selectedShape->centroid();
            Matrix3x3 t1 = Transformations::getTranslationMatrix(-c.x, -c.y);
            Matrix3x3 sc = Transformations::getScalingMatrix(s, s);
            Matrix3x3 t2 = Transformations::getTranslationMatrix(c.x, c.y);
            Matrix3x3 comp = t2 * sc * t1;
            for (auto& pt : e->selectedShape->vertices) pt = comp * pt;
            e->dragScaleFactor *= s;
        }

        e->lastMouseX = xpos; e->lastMouseY = ypos;
        return;
    }

    if (e->isDrawing && e->activeTool == Engine::TOOL_PEN) {
        float wx, wy;
        e->renderer.screenToWorld(xpos, ypos, wx, wy);
        e->currentShape.push_back(Point2D(wx, wy, 1.0f));
    }

    e->lastMouseX = xpos;
    e->lastMouseY = ypos;
}
```

Tracks mouse:
- **Panning** (`isPanning`): Adjusts world window by mouse delta ratio.
- **Dragging** (`isDragging && selectedShape`): Applies Move/Rotate/Scale transforms based on active tool.
- **Freehand draw** (`TOOL_PEN`): Appends vertices continuously.

---

### `InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)`

```cpp
// File: InputHandler.cpp
void InputHandler::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    Engine* e = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!e) return;

    if (ImGui::GetIO().WantCaptureMouse) return;

    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    float wx, wy;
    e->renderer.screenToWorld(xpos, ypos, wx, wy);

    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        e->isPanning = (action == GLFW_PRESS);
        return;
    }

    auto finishShape = [&]() {
        if (e->currentShape.size() >= 2) {
            Shape s;
            s.vertices = e->currentShape;
            s.closed = false;
            s.fill[0] = e->fillColor[0]; s.fill[1] = e->fillColor[1]; s.fill[2] = e->fillColor[2];
            s.stroke[0] = e->strokeColor[0]; s.stroke[1] = e->strokeColor[1]; s.stroke[2] = e->strokeColor[2];
            e->shapes.push_back(s);
        }
        e->isDrawing = false;
        e->currentShape.clear();
    };

    switch (e->activeTool) {
        case Engine::TOOL_SELECT:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->selectedShape = nullptr;
                Point2D pt(wx, wy, 1.0f);
                for (auto& s : e->shapes)
                    if (s.contains(pt)) { e->selectedShape = &s; break; }
            }
            break;

        case Engine::TOOL_DRAW_POLYGON:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->currentShape.push_back(Point2D(wx, wy, 1.0f));
                e->isDrawing = true;
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                if (e->currentShape.size() >= 3) {
                    Shape s;
                    s.vertices = e->currentShape;
                    s.closed = true;
                    s.fill[0] = e->fillColor[0]; s.fill[1] = e->fillColor[1]; s.fill[2] = e->fillColor[2];
                    s.stroke[0] = e->strokeColor[0]; s.stroke[1] = e->strokeColor[1]; s.stroke[2] = e->strokeColor[2];
                    e->shapes.push_back(s);
                }
                e->isDrawing = false;
                e->currentShape.clear();
            }
            break;

        case Engine::TOOL_DRAW_LINE:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->currentShape.push_back(Point2D(wx, wy, 1.0f));
                e->isDrawing = true;
            } else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
                if (!e->currentShape.empty()) {
                    e->currentShape.pop_back();
                    if (e->currentShape.empty()) e->isDrawing = false;
                }
            }
            break;

        case Engine::TOOL_PEN:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                e->isDrawing = true;
                e->currentShape.clear();
                e->currentShape.push_back(Point2D(wx, wy, 1.0f));
            } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                finishShape();
            }
            break;

        case Engine::TOOL_FILL:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                Point2D pt(wx, wy, 1.0f);
                bool hit = false;
                for (auto& s : e->shapes)
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
                        s.fill[0] = e->fillColor[0]; s.fill[1] = e->fillColor[1]; s.fill[2] = e->fillColor[2];
                        s.stroke[0] = e->strokeColor[0]; s.stroke[1] = e->strokeColor[1]; s.stroke[2] = e->strokeColor[2];
                        e->shapes.push_back(s);
                    }
                }
            }
            break;

        case Engine::TOOL_MOVE:
        case Engine::TOOL_ROTATE:
        case Engine::TOOL_SCALE:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                Point2D pt(wx, wy, 1.0f);
                e->selectedShape = nullptr;
                for (auto& s : e->shapes)
                    if (s.contains(pt)) { e->selectedShape = &s; break; }
                if (e->selectedShape) {
                    e->isDragging = true;
                    e->dragStartX = wx;
                    e->dragStartY = wy;
                    e->dragScaleFactor = 1.0f;
                    if (e->activeTool == Engine::TOOL_ROTATE) e->dragAngle = 0.0f;
                }
            } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
                e->isDragging = false;
            }
            break;

        case Engine::TOOL_SHAPE:
            if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
                std::vector<Point2D> verts;
                float s = e->shapeSize;
                switch (e->activeShapeType) {
                    case Engine::SHAPE_RECTANGLE: Transformations::generateRectangle(verts, wx, wy, s, s * 0.75f); break;
                    case Engine::SHAPE_CIRCLE:    Transformations::generateCircle(verts, wx, wy, s / 2); break;
                    case Engine::SHAPE_TRIANGLE:  Transformations::generateTriangle(verts, wx, wy, s); break;
                    case Engine::SHAPE_PENTAGON:  Transformations::generateRegularPolygon(verts, wx, wy, s / 2, 5); break;
                    case Engine::SHAPE_HEXAGON:   Transformations::generateRegularPolygon(verts, wx, wy, s / 2, 6); break;
                    case Engine::SHAPE_STAR:      Transformations::generateStar(verts, wx, wy, s / 2, s * 0.2f); break;
                }
                Shape shape;
                shape.vertices = verts;
                shape.fill[0] = e->fillColor[0]; shape.fill[1] = e->fillColor[1]; shape.fill[2] = e->fillColor[2];
                shape.stroke[0] = e->strokeColor[0]; shape.stroke[1] = e->strokeColor[1]; shape.stroke[2] = e->strokeColor[2];
                e->shapes.push_back(shape);
            }
            break;
    }
}
```

Dispatches per-tool logic:
- **SELECT:** Click-to-select using `Shape::contains()` (which calls `isPointInPolygon`).
- **DRAW_POLYGON:** Left-click adds vertices, right-click finishes polygon.
- **DRAW_LINE:** Left-click adds vertices, right-click removes last, Esc finishes.
- **PEN:** Click-drag draws freehand, release finishes.
- **FILL:** Click on shape to recolor, or click in empty area to fill enclosed region.
- **MOVE/ROTATE/SCALE:** Click on shape to select+drag, release to finish.
- **SHAPE:** Click to place a pre-defined shape type at cursor.

---

### `InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset)`

```cpp
// File: InputHandler.cpp
void InputHandler::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);

    Engine* engine = static_cast<Engine*>(glfwGetWindowUserPointer(window));
    if (!engine) return;

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
```

Zooms in/out around world center. Scroll up = 0.9x (zoom in), scroll down = 1.1x (zoom out).

---

### `Clipping::isPointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon)`

```cpp
// File: Clipping.cpp
bool Clipping::isPointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon) {
    int crossings = 0;
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        float xi = polygon[i].x, yi = polygon[i].y;
        float xj = polygon[(i + 1) % n].x, yj = polygon[(i + 1) % n].y;
        if ((yi > point.y) != (yj > point.y)) {
            float x_intersect = (xj - xi) * (point.y - yi) / (yj - yi) + xi;
            if (point.x < x_intersect)
                crossings++;
        }
    }
    return (crossings % 2) == 1;
}
```

Even-odd rule ray casting: Cast horizontal ray rightward from point, count edge crossings. Odd = inside, even = outside.

**Verification:** Click on a shape with Select tool → shape should highlight gold. Click outside → selection clears.

---

## Member 4 — OpenGL Rendering, State & Scan-Line Fill Algorithm

**File:** `src/Engine.cpp`, `src/Renderer.cpp`

---

### `void Renderer::render()`

```cpp
// File: Renderer.cpp
void Renderer::render() {
    glClearColor(0.15f, 0.18f, 0.20f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(engine.worldMinX, engine.worldMaxX, engine.worldMinY, engine.worldMaxY, -1.0, 1.0);
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
}
```

Main render loop:
1. Clear background (dark gray).
2. Set up orthographic projection from world coordinates.
3. Draw grid (call `drawGrid()`).
4. For each shape: clip to viewport → fill with scan-line → draw stroke outline.
5. Selected shape draws in gold (`1.0, 0.8, 0.2`).
6. If actively drawing: show in-progress preview (line/polygon vertices with rubber-band).

**Important:** GLFW profile must be `GLFW_OPENGL_COMPAT_PROFILE` (not CORE) to use `glBegin`/`glEnd`.

---

### `void Engine::update(float deltaTime)`

```cpp
// File: Engine.cpp
void Engine::update(float deltaTime) {
    (void)deltaTime;
}
```

Called every frame before `render()`. Placeholder for continuous logic (animations, blinking selections, etc.).

---

### `Renderer::scanLineFill(const std::vector<Point2D>& polygon)`

```cpp
// File: Renderer.cpp
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
```

Scan-line polygon fill:
1. Find yMin/yMax of polygon.
2. For each integer scan line between yMin and yMax:
   - Compute x-intersections of scan line with polygon edges.
   - Sort intersections left to right.
   - Draw horizontal lines between even-odd pairs.

**Verification:** Place a closed shape (e.g. Rectangle, Pentagon) — it should appear filled with its fill color.

---

## Member 5 — Line Clipping Algorithms

**File:** `src/Clipping.cpp`

---

### `Clipping::computeRegionCode(float x, float y, const BoundingBox& box)`

```cpp
// File: Clipping.cpp
int Clipping::computeRegionCode(float x, float y, const BoundingBox& box) {
    int code = INSIDE;
    if (x < box.xMin) code |= LEFT;
    if (x > box.xMax) code |= RIGHT;
    if (y < box.yMin) code |= BOTTOM;
    if (y > box.yMax) code |= TOP;
    return code;
}
```

Compute 4-bit outcode:

| Bit | Constant | Meaning |
|-----|----------|---------|
| 0 | `INSIDE = 0` | Inside all boundaries |
| 1 | `LEFT = 1` | `x < box.xMin` |
| 2 | `RIGHT = 2` | `x > box.xMax` |
| 4 | `BOTTOM = 4` | `y < box.yMin` |
| 8 | `TOP = 8` | `y > box.yMax` |

---

### `Clipping::cohenSutherlandClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box)`

```cpp
// File: Clipping.cpp
bool Clipping::cohenSutherlandClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    int code1 = computeRegionCode(p1.x, p1.y, box);
    int code2 = computeRegionCode(p2.x, p2.y, box);
    bool accept = false;

    while (true) {
        if (code1 == 0 && code2 == 0) {
            accept = true;
            break;
        } else if (code1 & code2) {
            break;
        } else {
            int codeOut = (code1 != 0) ? code1 : code2;
            float x, y;

            if (codeOut & TOP) {
                x = p1.x + (p2.x - p1.x) * (box.yMax - p1.y) / (p2.y - p1.y);
                y = box.yMax;
            } else if (codeOut & BOTTOM) {
                x = p1.x + (p2.x - p1.x) * (box.yMin - p1.y) / (p2.y - p1.y);
                y = box.yMin;
            } else if (codeOut & RIGHT) {
                y = p1.y + (p2.y - p1.y) * (box.xMax - p1.x) / (p2.x - p1.x);
                x = box.xMax;
            } else { // LEFT
                y = p1.y + (p2.y - p1.y) * (box.xMin - p1.x) / (p2.x - p1.x);
                x = box.xMin;
            }

            if (codeOut == code1) {
                p1.x = x; p1.y = y;
                code1 = computeRegionCode(p1.x, p1.y, box);
            } else {
                p2.x = x; p2.y = y;
                code2 = computeRegionCode(p2.x, p2.y, box);
            }
        }
    }

    return accept;
}
```

Cohen-Sutherland line clipping:
1. Compute region codes for both endpoints.
2. **Trivial accept:** both codes are 0 (inside).
3. **Trivial reject:** codes have common bit set (both outside same edge).
4. Otherwise: clip the outside endpoint to the appropriate edge, update its code, repeat.

---

### `Clipping::liangBarskyClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box)`

```cpp
// File: Clipping.cpp
bool Clipping::liangBarskyClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;

    float p[4] = { -dx, dx, -dy, dy };
    float q[4] = { p1.x - box.xMin, box.xMax - p1.x, p1.y - box.yMin, box.yMax - p1.y };

    float tEnter = 0.0f;
    float tExit = 1.0f;

    for (int i = 0; i < 4; ++i) {
        if (p[i] == 0.0f) {
            if (q[i] < 0.0f) return false;
        } else {
            float t = q[i] / p[i];
            if (p[i] < 0.0f)
                tEnter = std::max(tEnter, t);
            else
                tExit = std::min(tExit, t);
        }
    }

    if (tEnter > tExit) return false;

    float x1_clip = p1.x + tEnter * dx;
    float y1_clip = p1.y + tEnter * dy;
    float x2_clip = p1.x + tExit * dx;
    float y2_clip = p1.y + tExit * dy;

    p1.x = x1_clip; p1.y = y1_clip;
    p2.x = x2_clip; p2.y = y2_clip;
    return true;
}
```

Liang-Barsky parametric line clipping:
1. Parameterize line as `p = p1 + t * (p2 - p1)`.
2. For each edge, compute `p[i]` and `q[i]`.
3. Track `tEnter` (entering edges) and `tExit` (leaving edges).
4. If `tEnter > tExit`, line is fully outside.
5. Otherwise clip endpoints to `tEnter` / `tExit`.

**Verification for Member 5:** Currently these are called by `sutherlandHodgmanClipPolygon` during polygon-edge intersection. Once Member 1 and Member 4 implement their parts, clipped shapes should have clean boundaries at viewport edges.

---

## Appendix: Infrastructure Code (kept as-is in template)

These functions are **not** task assignments — they are infrastructure that remains in the template:

| File | Functions |
|---|---|
| `Transformations.cpp` | `Matrix3x3::Matrix3x3()`, `getReflectionMatrix`, `mirrorShape`, `generateRectangle`, `generateCircle`, `generateTriangle`, `generateRegularPolygon`, `generateStar` |
| `Clipping.cpp` | `cross2D`, `dot2D`, `sub2D`, `segSegIntersect`, `distSq`, `normAngle`, `findEnclosedRegion` |
| `Renderer.cpp` | `Renderer()`, `drawGrid`, `screenToWorld` |
| `Engine.cpp` | All (constructor, `initialize`, `run`, `shutdown`, `setWorldWindow`, `setViewport`) |
| `InputHandler.cpp` | `framebufferSizeCallback`, `windowFocusCallback`, `cursorEnterCallback` |
| `UIBuilder.cpp` | All (UI layout, buttons, menus, status bar) |
| `Shape.cpp` | All (delegates to task functions) |
| `main.cpp` | Entry point |
