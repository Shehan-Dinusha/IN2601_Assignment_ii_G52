# IN2601 Assignment II — Implementation Reference for Members

This document is the **single source of truth** for implementing your assigned tasks.  
Give this file to your AI agent and say: *"Implement Task N exactly as specified. Do not modify any other files or code outside the task description."*

---

## Table of Contents

1. [Project Overview](#1-project-overview)
2. [Architecture Summary](#2-architecture-summary)
3. [Task 1 — Cohen-Sutherland Line Clipping](#3-task-1--cohen-sutherland-line-clipping)
4. [Task 2 — Liang-Barsky Line Clipping](#4-task-2--liang-barsky-line-clipping)
5. [Task 3 — Sutherland-Hodgman Polygon Clipping](#5-task-3--sutherland-hodgman-polygon-clipping)
6. [Task 4 — Undo / Redo](#6-task-4--undo--redo)
7. [Task 5 — File Save / Load](#7-task-5--file-save--load)
8. [Task 6 — Grid Snapping](#8-task-6--grid-snapping)
9. [Task 7 — Selection Box (Drag-Select)](#9-task-7--selection-box-drag-select)
10. [Task 8 — Keyboard Shortcuts](#10-task-8--keyboard-shortcuts)
11. [Task 9 — Shape Manipulation via UI](#11-task-9--shape-manipulation-via-ui)

---

## 1. Project Overview

### Directory Structure
```
vendor/         ← Third-party code — DO NOT MODIFY
├── glad/       GLAD (OpenGL loader)
├── glfw/       GLFW (windowing)
└── imgui/      Dear ImGui (UI framework)

include/        ← User headers — you will modify these
├── Clipping.h
├── Engine.h
├── InputHandler.h
├── Renderer.h
├── Shape.h
├── Transformations.h
└── UIBuilder.h

src/            ← User source — you will modify these
├── main.cpp
├── Clipping.cpp
├── Engine.cpp
├── InputHandler.cpp
├── Renderer.cpp
├── Shape.cpp
├── Transformations.cpp
└── UIBuilder.cpp

lib/            ← Pre-compiled libraries — DO NOT MODIFY
.vscode/        ← Build config — DO NOT MODIFY
```

### Build Command (also `Ctrl+Shift+B` in VS Code)
```bash
g++ -g -std=c++17 -Iinclude -Ivendor/glad -Ivendor/glfw -Ivendor/imgui -Llib src/*.cpp vendor/glad/glad.c vendor/imgui/*.cpp -lglfw3dll -lfreeglut -lopengl32 -lglu32 -o app.exe
```

### Coding Conventions
- C++17 standard
- Class naming: PascalCase (`Shape`, `Engine`, `Renderer`, ...)
- Method/function naming: camelCase (`centroid()`, `contains()`, `screenToWorld()`)
- Member variables: camelCase (`worldMinX`, `selectedShape`, `activeTool`)
- Enums: `UPPER_CASE` (`TOOL_SELECT`, `SHAPE_RECTANGLE`)
- Indentation: 4 spaces, no tabs
- `#include` your own headers with `"quotes"`, system headers with `<angles>`
- Include paths: `"Engine.h"` (no relative paths — `-Iinclude` covers it)
- Keep vendor includes: `"glad/glad.h"`, `<GLFW/glfw3.h>`, `"imgui.h"`

---

## 2. Architecture Summary

### Data Flow
```
User Input (GLFW) → InputHandler (static callbacks) → Engine state modification
                                                              ↓
Engine::run() loop → Renderer::render() draws shapes from Engine state
                   → UIBuilder::buildUI() reads/writes Engine state via ImGui
```

### Class Responsibilities
| Class | Role | Key data accessed on Engine |
|---|---|---|
| `Engine` | Init, run, shutdown, owns all state | — |
| `Renderer` | OpenGL draw calls | `shapes`, `selectedShape`, `currentShape`, `worldMin/Max`, `windowWidth/Height`, `showGrid` |
| `UIBuilder` | ImGui widgets | `shapes`, `selectedShape`, `activeTool`, `fillColor`, `strokeColor`, `shapeSize` |
| `InputHandler` | GLFW → tool dispatch | Everything via `Engine*` pointer |
| `Shape` | Vertex data + convenience methods | — |
| `Clipping` | Geometry algorithms (static) | — |
| `Transformations` | Matrix + shape generators (static) | — |

### Key Conventions for Integration
- **To add a new Engine member variable**: declare it in `Engine.h` private section, initialize in `Engine.cpp` constructor
- **To add a new tool**: add value to `Engine::Tool` enum, add button in `UIBuilder.cpp`
- **To add render logic**: add code in `Renderer::render()`
- **To react to key press**: add case in `InputHandler::keyCallback()`
- **Engine.h uses forward declaration** `struct GLFWwindow;` — do not add `#include <GLFW/glfw3.h>` to Engine.h
- **ImGui includes** only in `.cpp` files, never in `.h` files

---

## 3. Task 1 — Cohen-Sutherland Line Clipping

### Files to Modify
- `include/Clipping.h` — no changes needed (declaration exists)
- `src/Clipping.cpp` — implement `computeRegionCode()` and `cohenSutherlandClipLine()`

### Files NOT to Modify
- Any file outside `Clipping.h` / `Clipping.cpp`

### Current State (Stub)
```cpp
int Clipping::computeRegionCode(float x, float y, const BoundingBox& box) {
    int code = INSIDE;
    // TODO
    return code;
}

bool Clipping::cohenSutherlandClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    // TODO
    return true; // Placeholder — always says visible
}
```

### Implementation Detail

#### `computeRegionCode(x, y, box)`

Compute a 4-bit outcode where each bit represents being outside a boundary:

| Bit | Constant | Meaning |
|---|---|---|
| 0 | `INSIDE = 0` | Inside all boundaries |
| 1 | `LEFT = 1` | `x < box.xMin` |
| 2 | `RIGHT = 2` | `x > box.xMax` |
| 4 | `BOTTOM = 4` | `y < box.yMin` |
| 8 | `TOP = 8` | `y > box.yMax` |

Pseudo-code:
```
code = INSIDE
if x < box.xMin: code |= LEFT
if x > box.xMax: code |= RIGHT
if y < box.yMin: code |= BOTTOM
if y > box.yMax: code |= TOP
return code
```

#### `cohenSutherlandClipLine(p1, p2, box)`

Pseudo-code:
```
code1 = computeRegionCode(p1.x, p1.y, box)
code2 = computeRegionCode(p2.x, p2.y, box)
accept = false

loop forever:
    if (code1 == 0 && code2 == 0):          // trivial accept
        accept = true; break
    else if (code1 & code2) != 0:            // trivial reject
        break
    else:
        // pick outside endpoint
        codeOut = (code1 != 0) ? code1 : code2
        x, y = intersection of line with edge indicated by codeOut
        // intersection calculation:
        if codeOut & TOP:    x = p1.x + (p2.x-p1.x)*(box.yMax-p1.y)/(p2.y-p1.y); y = box.yMax
        if codeOut & BOTTOM: x = p1.x + (p2.x-p1.x)*(box.yMin-p1.y)/(p2.y-p1.y); y = box.yMin
        if codeOut & RIGHT:  y = p1.y + (p2.y-p1.y)*(box.xMax-p1.x)/(p2.x-p1.x); x = box.xMax
        if codeOut & LEFT:   y = p1.y + (p2.y-p1.y)*(box.xMin-p1.x)/(p2.x-p1.x); x = box.xMin
        
        if codeOut == code1:
            p1.x = x; p1.y = y
            code1 = computeRegionCode(p1.x, p1.y, box)
        else:
            p2.x = x; p2.y = y
            code2 = computeRegionCode(p2.x, p2.y, box)

return accept
```

### Verification
Run the program, the clipping stubs are currently called by `Renderer::render()` via `sutherlandHodgmanClipPolygon()` which calls line clipping. For this task, just verify that the function compiles and returns correct region codes. A simple way: add a temporary `printf` or check via debugger.

---

## 4. Task 2 — Liang-Barsky Line Clipping

### Files to Modify
- `src/Clipping.cpp` — implement `liangBarskyClipLine()`

### Files NOT to Modify
- Any file outside `Clipping.cpp`
- Do not modify `computeRegionCode()` (already implemented in Task 1)

### Current State (Stub)
```cpp
bool Clipping::liangBarskyClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    // TODO
    return true; // Placeholder
}
```

### Implementation Detail

```
dx = p2.x - p1.x
dy = p2.y - p1.y
p = [-dx, dx, -dy, dy]
q = [p1.x - box.xMin, box.xMax - p1.x, p1.y - box.yMin, box.yMax - p1.y]

tEnter = 0.0
tExit  = 1.0

for i = 0..3:
    if p[i] == 0:
        if q[i] < 0: return false   // parallel and outside
        // else parallel and inside — skip
    else:
        t = q[i] / p[i]
        if p[i] < 0:
            tEnter = max(tEnter, t)
        else:
            tExit  = min(tExit, t)

if tEnter > tExit: return false

// Clipped points:
x1_clip = p1.x + tEnter * dx
y1_clip = p1.y + tEnter * dy
x2_clip = p1.x + tExit  * dx
y2_clip = p1.y + tExit  * dy

p1.x = x1_clip; p1.y = y1_clip
p2.x = x2_clip; p2.y = y2_clip
return true
```

### Verification
Same as Task 1 — currently not called by any user-facing code, but the function will be used when Sutherland-Hodgman is correctly implemented.

---

## 5. Task 3 — Sutherland-Hodgman Polygon Clipping

### Files to Modify
- `src/Clipping.cpp` — implement `sutherlandHodgmanClipPolygon()`

### Files NOT to Modify
- Any file outside `Clipping.cpp`

### Current State (Stub)
```cpp
bool Clipping::sutherlandHodgmanClipPolygon(std::vector<Point2D>& polygon, const BoundingBox& box) {
    // TODO
    return true; // Placeholder
}
```

### Implementation Detail

This function clips a polygon against the 4 edges of `box` (left, right, bottom, top) sequentially.

Helper function (add as a static function in Clipping.cpp):
```cpp
// Clip polygon 'poly' against a single clip edge defined by:
//   orientation = 0 (left: x >= box.xMin)
//   orientation = 1 (right: x <= box.xMax)
//   orientation = 2 (bottom: y >= box.yMin)
//   orientation = 3 (top: y <= box.yMax)
static void clipAgainstEdge(
    const std::vector<Point2D>& poly,
    std::vector<Point2D>& out,
    const BoundingBox& box,
    int edge
);
```

For each clip edge, determine which side is "inside":
- Left (edge 0):   `p.x >= box.xMin`
- Right (edge 1):  `p.x <= box.xMax`
- Bottom (edge 2): `p.y >= box.yMin`
- Top (edge 3):    `p.y <= box.yMax`

For each consecutive edge (current → next) in the polygon:
```
let cur = poly[i]
let nxt = poly[(i+1) % poly.size()]

curInside = isInside(cur, edge, box)
nxtInside = isInside(nxt, edge, box)

if curInside:
    out.push_back(cur)
    if !nxtInside:
        out.push_back(intersection(cur, nxt, edge, box))
else:
    if nxtInside:
        out.push_back(intersection(cur, nxt, edge, box))
        out.push_back(nxt)
    // else both outside: add nothing
```

Computing intersection point for each edge:
- Left (x=box.xMin):   y = cur.y + (nxt.y-cur.y)*(box.xMin-cur.x)/(nxt.x-cur.x)
- Right (x=box.xMax):  y = cur.y + (nxt.y-cur.y)*(box.xMax-cur.x)/(nxt.x-cur.x)
- Bottom (y=box.yMin): x = cur.x + (nxt.x-cur.x)*(box.yMin-cur.y)/(nxt.y-cur.y)
- Top (y=box.yMax):    x = cur.x + (nxt.x-cur.x)*(box.yMax-cur.y)/(nxt.y-cur.y)

Main function pseudo-code:
```
if polygon.size() < 3: return false

output = polygon
for edge = 0..3:
    input = output
    output.clear()
    clipAgainstEdge(input, output, box, edge)
    if output.size() < 3: return false

polygon = output
return true
```

### Integration Context
This function is called by `Renderer::render()` at line:
```cpp
std::vector<Point2D> clipped = shape.vertices;
Clipping::sutherlandHodgmanClipPolygon(clipped, clipBox);
```
When implemented correctly, shapes visible on screen will be properly clipped to the world viewport boundaries. Currently the stub `return true` means shapes outside the view are still rendered partially.

### Verification
1. Run the program
2. Draw a shape that extends beyond the viewable area
3. Zoom out (scroll) — the shape should now be clipped at the viewport edges instead of wrapping around

---

## 6. Task 4 — Undo / Redo

### Files to Modify
- `include/Engine.h` — add `undoStack`, `redoStack`, `pushUndo()`
- `src/Engine.cpp` — implement `pushUndo()`, initialize stacks
- `src/InputHandler.cpp` — call `pushUndo()` before each destructive operation, add Ctrl+Z / Ctrl+Y handling
- `src/UIBuilder.cpp` — update Help menu with undo/redo instructions

### Files NOT to Modify
- `Renderer.cpp`, `Shape.cpp`, `Clipping.cpp`, `Transformations.cpp`
- Any vendor file

### Implementation Detail

#### In `Engine.h`:
Add member variables in the private section:
```cpp
std::vector<std::vector<Shape>> undoStack;
std::vector<std::vector<Shape>> redoStack;
```

Add methods:
```cpp
void pushUndo();    // saves current shapes to undoStack, clears redoStack
void undo();        // pops undoStack → redoStack, restores shapes
void redo();        // pops redoStack → undoStack, restores shapes
```

#### In `Engine.cpp`:
Constructor: no special init needed (stacks default-construct empty).

Implement:
```cpp
void Engine::pushUndo() {
    undoStack.push_back(shapes);
    redoStack.clear();
}

void Engine::undo() {
    if (undoStack.empty()) return;
    redoStack.push_back(shapes);
    shapes = undoStack.back();
    undoStack.pop_back();
    // Clear selectedShape pointer (may be dangling)
    selectedShape = nullptr;
}

void Engine::redo() {
    if (redoStack.empty()) return;
    undoStack.push_back(shapes);
    shapes = redoStack.back();
    redoStack.pop_back();
    selectedShape = nullptr;
}
```

#### In `InputHandler.cpp`:
Call `e->pushUndo()` BEFORE each operation that modifies shapes:
- `TOOL_DRAW_POLYGON` right-click finish (line 504)
- `TOOL_PEN` finishShape (line 535)
- `TOOL_FILL` (line 545)
- `TOOL_MOVE` / `TOOL_ROTATE` / `TOOL_SCALE` (line 583) — only on mousedown, not during drag
- `TOOL_SHAPE` place (line 606)
- `keyCallback` Esc finish line (line 650)
- Delete button press (in UIBuilder)

Add Ctrl+Z / Ctrl+Y handling in `keyCallback`:
```cpp
// Inside keyCallback, add before the ESC check:
if ((key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL)) || key == GLFW_KEY_F1) {
    // Ctrl+Z: undo (but F1 example — use GLFW_MOD_CONTROL)
}
```
For keyCallback mods parameter check:
```cpp
if (key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL)) {
    e->undo();
    return;
}
if (key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL)) {
    e->redo();
    return;
}
```

#### In `UIBuilder.cpp`:
In the View menu (or Help menu), add menu items:
```cpp
if (ImGui::MenuItem("Undo", "Ctrl+Z"))  engine.undo();
if (ImGui::MenuItem("Redo", "Ctrl+Y"))  engine.redo();
```
Place these in the Edit menu or File menu (create an Edit menu if needed).

### Verification
1. Add a shape
2. Press Ctrl+Z — shape disappears
3. Press Ctrl+Y — shape reappears
4. Add another shape, press Ctrl+Z twice, then add a different shape — verify the first redo history is cleared

---

## 7. Task 5 — File Save / Load

### Files to Modify
- `include/Engine.h` — add `saveShapes()`, `loadShapes()` methods
- `src/Engine.cpp` — implement save/load
- `src/UIBuilder.cpp` — add "Save" and "Open" menu items with file dialog

### Files NOT to Modify
- `Renderer.cpp`, `InputHandler.cpp`, `Shape.cpp`, `Clipping.cpp`, `Transformations.cpp`

### Implementation Detail

#### File Format (Simple Text)
Write a simple custom format. Example `.cad` format:
```
# shapes count
6

# shape 0
4              ← vertex count
3              ← closed (1) or open (0)
0.6 0.6 1.0    ← fill R G B
1.0 1.0 1.0    ← stroke R G B
10.0 20.0      ← vertex x y
30.0 40.0
50.0 60.0
70.0 80.0

# shape 1
3
1
...
```

#### In `Engine.h`:
Add:
```cpp
void saveShapes(const char* filename);
void loadShapes(const char* filename);
```

#### In `Engine.cpp`:

Implement save:
```cpp
void Engine::saveShapes(const char* filename) {
    FILE* f = fopen(filename, "w");
    if (!f) return;
    fprintf(f, "%zu\n", shapes.size());
    for (auto& s : shapes) {
        fprintf(f, "%zu %d\n", s.vertices.size(), s.closed ? 1 : 0);
        fprintf(f, "%f %f %f\n", s.fill[0], s.fill[1], s.fill[2]);
        fprintf(f, "%f %f %f\n", s.stroke[0], s.stroke[1], s.stroke[2]);
        for (auto& pt : s.vertices)
            fprintf(f, "%f %f\n", pt.x, pt.y);
    }
    fclose(f);
}
```

Implement load:
```cpp
void Engine::loadShapes(const char* filename) {
    FILE* f = fopen(filename, "r");
    if (!f) return;
    size_t count;
    fscanf(f, "%zu", &count);
    shapes.clear();
    selectedShape = nullptr;
    currentShape.clear();
    isDrawing = false;
    for (size_t i = 0; i < count; ++i) {
        Shape s;
        size_t vertCount;
        int closedVal;
        fscanf(f, "%zu %d", &vertCount, &closedVal);
        s.closed = (closedVal != 0);
        fscanf(f, "%f %f %f", &s.fill[0], &s.fill[1], &s.fill[2]);
        fscanf(f, "%f %f %f", &s.stroke[0], &s.stroke[1], &s.stroke[2]);
        s.vertices.resize(vertCount);
        for (size_t j = 0; j < vertCount; ++j)
            fscanf(f, "%f %f", &s.vertices[j].x, &s.vertices[j].y);
        shapes.push_back(s);
    }
    fclose(f);
}
```

#### In `UIBuilder.cpp`:
In the File menu, add:
```cpp
if (ImGui::MenuItem("Save", "Ctrl+S")) {
    nfdchar_t* path = NULL;
    // Use a simple approach: call a file dialog or just save to a fixed path
    engine.saveShapes("save.cad");
}
if (ImGui::MenuItem("Open", "Ctrl+O")) {
    engine.loadShapes("save.cad");
}
```

For a proper file dialog, you can add the `tinyfiledialogs` library or use `ImGuiFileDialog`. For the assignment, a fixed filename `"save.cad"` is acceptable.

### Verification
1. Create several shapes with different colors
2. File → Save
3. File → Clear All
4. File → Open — shapes should reappear exactly as before

---

## 8. Task 6 — Grid Snapping

### Files to Modify
- `src/InputHandler.cpp` — snap mouse coordinates to grid before adding vertices

### Files NOT to Modify
- Any header file, Renderer.cpp, UIBuilder.cpp, Clipping.cpp, Transformations.cpp

### Implementation Detail

Add a helper lambda at the top of `mouseButtonCallback`:
```cpp
auto snapToGrid = [](float val, float step) -> float {
    return std::round(val / step) * step;
};
```

Then use it whenever you add a vertex or place a shape. Find these locations in `mouseButtonCallback`:

1. **TOOL_DRAW_POLYGON** left-click (line 500-502):
```cpp
float sx = snapToGrid(wx, 10.0f);
float sy = snapToGrid(wy, 10.0f);
e->currentShape.push_back(Point2D(sx, sy, 1.0f));
```

2. **TOOL_DRAW_LINE** left-click (line 518-520):
```cpp
float sx = snapToGrid(wx, 10.0f);
float sy = snapToGrid(wy, 10.0f);
e->currentShape.push_back(Point2D(sx, sy, 1.0f));
```

3. **TOOL_PEN** left-click press (line 530-534):
```cpp
float sx = snapToGrid(wx, 10.0f);
float sy = snapToGrid(wy, 10.0f);
e->currentShape.push_back(Point2D(sx, sy, 1.0f));
```

4. **TOOL_SHAPE** place (line 595-600):
```cpp
float sx = snapToGrid(wx, 10.0f);
float sy = snapToGrid(wy, 10.0f);
// use sx, sy instead of wx, wy in the generators
```

Also consider adding a `bool snapToGrid` toggle in Engine.h (like `showGrid`) and a menu item in View → UIBuilder.cpp.

### Verification
1. Draw a polygon — vertices should snap to the nearest multiple of 10
2. Draw a line — endpoints should snap
3. Place a shape — center should snap

---

## 9. Task 7 — Selection Box (Drag-Select)

### Files to Modify
- `include/Engine.h` — add `selectionBox` rectangle state, `isSelecting` flag
- `src/Engine.cpp` — initialize
- `src/InputHandler.cpp` — track rectangle on drag, select on release
- `src/Renderer.cpp` — draw selection rectangle
- `src/UIBuilder.cpp` — show selected count

### Files NOT to Modify
- Clipping.cpp, Transformations.cpp, Shape.cpp (unless needed for bounds check)

### Implementation Detail

#### In `Engine.h`:
Add to private section:
```cpp
bool isSelecting;
float selBoxX1, selBoxY1, selBoxX2, selBoxY2;
```

#### In `Engine.cpp` constructor:
```cpp
isSelecting(false),
selBoxX1(0), selBoxY1(0), selBoxX2(0), selBoxY2(0),
```

#### In `InputHandler.cpp`:

In `mouseButtonCallback`, add selection logic for `TOOL_SELECT`:

Modify the `TOOL_SELECT` case:
```cpp
case Engine::TOOL_SELECT:
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        e->isSelecting = true;
        e->selBoxX1 = e->selBoxX2 = wx;
        e->selBoxY1 = e->selBoxY2 = wy;
        e->selectedShape = nullptr;
        // Single-click select (no drag yet)
        for (auto& s : e->shapes)
            if (s.contains(Point2D(wx, wy, 1.0f))) { e->selectedShape = &s; break; }
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        if (e->isSelecting) {
            // Check if drag was significant (selection box)
            float dx = e->selBoxX2 - e->selBoxX1;
            float dy = e->selBoxY2 - e->selBoxY1;
            if (fabs(dx) > 1.0f || fabs(dy) > 1.0f) {
                // Box select: find all shapes whose centroid lies within the box
                float minX = std::min(e->selBoxX1, e->selBoxX2);
                float maxX = std::max(e->selBoxX1, e->selBoxX2);
                float minY = std::min(e->selBoxY1, e->selBoxY2);
                float maxY = std::max(e->selBoxY1, e->selBoxY2);
                // For simplicity, just select the first shape inside the box area
                e->selectedShape = nullptr;
                for (auto& s : e->shapes) {
                    Point2D c = s.centroid();
                    if (c.x >= minX && c.x <= maxX && c.y >= minY && c.y <= maxY) {
                        e->selectedShape = &s;
                        break;
                    }
                }
            }
            e->isSelecting = false;
        }
    }
    break;
```

In `cursorPositionCallback`, add to update selection box:
```cpp
// At the top of cursorPositionCallback, after the pen drawing check:
if (e->isSelecting) {
    float wx, wy;
    e->renderer.screenToWorld(xpos, ypos, wx, wy);
    e->selBoxX2 = wx;
    e->selBoxY2 = wy;
}
```

#### In `Renderer.cpp`:

In `render()`, after drawing all shapes, draw the selection rectangle:
```cpp
if (engine.isSelecting) {
    float minX = std::min(engine.selBoxX1, engine.selBoxX2);
    float maxX = std::max(engine.selBoxX1, engine.selBoxX2);
    float minY = std::min(engine.selBoxY1, engine.selBoxY2);
    float maxY = std::max(engine.selBoxY1, engine.selBoxY2);
    glColor3f(0.3f, 0.6f, 1.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2f(minX, minY);
    glVertex2f(maxX, minY);
    glVertex2f(maxX, maxY);
    glVertex2f(minX, maxY);
    glEnd();
    // Semi-transparent fill
    glColor4f(0.3f, 0.6f, 1.0f, 0.1f);
    glBegin(GL_QUADS);
    glVertex2f(minX, minY);
    glVertex2f(maxX, minY);
    glVertex2f(maxX, maxY);
    glVertex2f(minX, maxY);
    glEnd();
}
```

Note: `glColor4f` requires `GL_BLEND` to be enabled. Add at the top of `render()`:
```cpp
glEnable(GL_BLEND);
glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
```

### Verification
1. Select tool → click-drag on empty canvas → a blue rectangle appears
2. Release → nearest shape inside the rectangle gets selected (highlighted gold)

---

## 10. Task 8 — Keyboard Shortcuts

### Files to Modify
- `src/InputHandler.cpp` — add hotkeys to `keyCallback()`
- `src/UIBuilder.cpp` — update Help menu with shortcut list

### Files NOT to Modify
- Any header, Renderer.cpp, Clipping.cpp, Transformations.cpp

### Implementation Detail

#### In `keyCallback()` (InputHandler.cpp):

Add these checks **before** the Esc check. The `mods` parameter contains modifier flags.

```cpp
// In keyCallback(), before the Esc handling:
if (key == GLFW_KEY_Z && (mods & GLFW_MOD_CONTROL)) { engine->undo(); return; }
if (key == GLFW_KEY_Y && (mods & GLFW_MOD_CONTROL)) { engine->redo(); return; }
if (key == GLFW_KEY_S && (mods & GLFW_MOD_CONTROL)) { engine->saveShapes("save.cad"); return; }
if (key == GLFW_KEY_O && (mods & GLFW_MOD_CONTROL)) { engine->loadShapes("save.cad"); return; }

// Tool hotkeys (no modifier):
if (key == GLFW_KEY_1) { engine->activeTool = Engine::TOOL_SELECT; return; }
if (key == GLFW_KEY_2) { engine->activeTool = Engine::TOOL_DRAW_POLYGON; return; }
if (key == GLFW_KEY_3) { engine->activeTool = Engine::TOOL_DRAW_LINE; return; }
if (key == GLFW_KEY_4) { engine->activeTool = Engine::TOOL_PEN; return; }
if (key == GLFW_KEY_5) { engine->activeTool = Engine::TOOL_FILL; return; }
if (key == GLFW_KEY_M) { engine->activeTool = Engine::TOOL_MOVE; return; }
if (key == GLFW_KEY_R) { engine->activeTool = Engine::TOOL_ROTATE; return; }
if (key == GLFW_KEY_S && !(mods & GLFW_MOD_CONTROL)) { engine->activeTool = Engine::TOOL_SCALE; return; }

// Delete key
if (key == GLFW_KEY_DELETE || key == GLFW_KEY_BACKSPACE) {
    if (engine->selectedShape) {
        engine->pushUndo();
        for (auto it = engine->shapes.begin(); it != engine->shapes.end(); ++it) {
            if (&*it == engine->selectedShape) { engine->shapes.erase(it); break; }
        }
        engine->selectedShape = nullptr;
    }
    return;
}
```

#### In `UIBuilder.cpp`:
Update Help menu text:
```cpp
ImGui::Text("Shortcuts:");
ImGui::Text("  Ctrl+Z=Undo  Ctrl+Y=Redo  Ctrl+S=Save  Ctrl+O=Open");
ImGui::Text("  1=Select  2=Polygon  3=Line  4=Pen  5=Fill");
ImGui::Text("  M=Move  R=Rotate  S=Scale  Del=Delete");
```

### Verification
1. Create a shape → press Del → shape deleted
2. Press 2 → Polygon tool activates (button highlights blue)
3. Press M → Move tool activates
4. Ctrl+Z after deleting → shape comes back

---

## 11. Task 9 — Shape Manipulation via UI

### Files to Modify
- `include/Engine.h` — no changes needed (uses existing state)
- `src/UIBuilder.cpp` — add numeric input fields in the Selection section

### Files NOT to Modify
- Renderer.cpp, InputHandler.cpp, Clipping.cpp, Transformations.cpp

### Implementation Detail

#### In `buildUI()` (UIBuilder.cpp):

In the `if (engine.selectedShape)` block, add numeric fields **after** the Mirror buttons:

```cpp
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
        engine.pushUndo();
        for (auto it = engine.shapes.begin(); it != engine.shapes.end(); ++it) {
            if (&*it == engine.selectedShape) { engine.shapes.erase(it); break; }
        }
        engine.selectedShape = nullptr;
    }

    // --- Numeric manipulation fields ---
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::TextUnformatted("Transform");

    float pos[2] = { engine.selectedShape->centroid().x, engine.selectedShape->centroid().y };
    if (ImGui::InputFloat2("Position", pos)) {
        Point2D c = engine.selectedShape->centroid();
        float dx = pos[0] - c.x;
        float dy = pos[1] - c.y;
        engine.selectedShape->translate(dx, dy);
    }

    static float rotationAngle = 0.0f;
    if (ImGui::InputFloat("Rotation", &rotationAngle)) {
        Point2D c = engine.selectedShape->centroid();
        engine.selectedShape->rotate(rotationAngle * 3.14159265f / 180.0f, c);
    }

    static float scaleVal = 1.0f;
    if (ImGui::InputFloat("Scale", &scaleVal)) {
        Point2D c = engine.selectedShape->centroid();
        Matrix3x3 t1 = Transformations::getTranslationMatrix(-c.x, -c.y);
        Matrix3x3 sc = Transformations::getScalingMatrix(scaleVal, scaleVal);
        Matrix3x3 t2 = Transformations::getTranslationMatrix(c.x, c.y);
        Matrix3x3 comp = t2 * sc * t1;
        for (auto& pt : engine.selectedShape->vertices) pt = comp * pt;
        scaleVal = 1.0f; // reset to avoid compounding
    }
}
```

Note: `pushUndo()` is called when the Delete button is pressed. This assumes Task 4 (undo/redo) is already implemented. If not, the `engine.pushUndo()` call can be omitted or guarded with `#ifdef`.

### Verification
1. Select a shape → Transform section appears in the panel
2. Change Position values → shape moves
3. Change Rotation → shape rotates (input in degrees)
4. Change Scale → shape resizes

---

## Appendix — Common Pitfalls

| Issue | Solution |
|---|---|
| `undefined reference to 'Clipping::xxx'` | You added a new static method declaration in `.h` without defining it in `.cpp`. Check both files. |
| `glfw3.dll` not found at runtime | Copy `glfw3.dll` from `lib/` to the project root next to `app.exe` |
| ImGui doesn't respond to clicks | Verify `ImGui_ImplGlfw_*Callback()` forwarding calls are present in all 7 callbacks |
| `Permission denied` when building | Close running `app.exe` or rename old exe |
| Shapes not showing or flickering | Likely a Sutherland-Hodgman issue — check if `clipped.size() < 3` is causing `continue` in render loop |
| `selectedShape` becomes dangling pointer | After `shapes.erase()` or `shapes.clear()`, always set `selectedShape = nullptr` |
