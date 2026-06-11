# IN2601 Assignment II — Member Task Assignments

Every function in the project is assigned to exactly one of the five members.
Existing assignments from `COMPLETE_PROJECT_REFERENCE.md` and `TODO.md` are preserved;
new features and infrastructure are divided equally on top.

---

## Member 1 — Core Math, Matrix Algebra, Shape Generators & Polygon Clipping

**234223A — Shehan Weerasinghe**

**Files:** `Transformations.cpp`, `Clipping.cpp`
**Category:** Primitive geometry, matrix construction, clipping infrastructure

### Matrix & Transform (Transformations.cpp)
- `Matrix3x3::Matrix3x3()` — identity constructor
- `Matrix3x3::operator*(const Matrix3x3&)` — 3×3 matrix multiplication
- `Matrix3x3::operator*(const Point2D&)` — homogeneous transform of a point
- `Transformations::getTranslationMatrix(tx, ty)` — [1 0 tx; 0 1 ty; 0 0 1]
- `Transformations::getScalingMatrix(sx, sy)` — [sx 0 0; 0 sy 0; 0 0 1]
- `Transformations::getRotationMatrix(angleRadians)` — [cos -sin 0; sin cos 0; 0 0 1]
- `Transformations::getReflectionMatrix(mirrorX, mirrorY)` — axis-aligned reflection

### Shape Generators (Transformations.cpp)
- `Transformations::generateRectangle(out, cx, cy, w, h)`
- `Transformations::generateCircle(out, cx, cy, radius, segments)`
- `Transformations::generateTriangle(out, cx, cy, size)`
- `Transformations::generateRegularPolygon(out, cx, cy, radius, sides)`
- `Transformations::generateStar(out, cx, cy, outerRadius, innerRadius)`

### Geometry Helpers (Clipping.cpp)
- `cross2D(a, b)` — 2D cross product
- `dot2D(a, b)` — 2D dot product
- `sub2D(a, b)` — 2D subtraction
- `segSegIntersect(a, b, c, d, out)` — segment intersection test
- `distSq(a, b)` — squared distance
- `normAngle(a)` — normalize angle to [0, 2π)
- `findEnclosedRegion(out, click, shapeVertices, shapeClosed)` — face detection from intersecting segments

### Polygon Clipping (Clipping.cpp)
- `clipAgainstEdge(poly, out, box, edge)` — single-edge Sutherland–Hodgman helper
- `Clipping::sutherlandHodgmanClipPolygon(polygon, box)` — full polygon clip against rectangle

---

## Member 2 — Applied Geometry & Shape Manipulation

**234225G — Kaveen Wettewa**

**Files:** `Transformations.cpp`, `Shape.cpp`
**Category:** Shape-level transformation wrappers

### Transform Operations (Transformations.cpp)
- `Transformations::translateShape(shape, tx, ty)` — translate all vertices
- `Transformations::scaleShape(shape, sx, sy)` — scale all vertices about origin
- `Transformations::rotateShapeAroundOrigin(shape, angleRadians)` — rotate about (0,0)
- `Transformations::rotateShapeAroundPivot(shape, pivot, angleRadians)` — rotate about arbitrary point
- `Transformations::mirrorShape(shape, horizontal, vertical)` — reflect about centroid

### Shape Methods (Shape.cpp)
- `Shape::centroid()` — vertex average of the shape
- `Shape::contains(point)` — delegates to `Clipping::isPointInPolygon`
- `Shape::translate(dx, dy)` — delegates to `Transformations::translateShape`
- `Shape::rotate(angleRadians, pivot)` — delegates to `Transformations::rotateShapeAroundPivot`
- `Shape::mirror(horizontal, vertical)` — delegates to `Transformations::mirrorShape`

---

## Member 3 — Engine Input, Camera Controls & Hit-Testing

**234222U — Kaweesha Weerasinghe**

**Files:** `InputHandler.cpp`, `Clipping.cpp`, `main.cpp`
**Category:** GLFW callbacks, pan/zoom, selection logic, hotkeys

### Callbacks (InputHandler.cpp)
- `InputHandler::cursorPositionCallback(window, xpos, ypos)` — panning, dragging (Move/Rotate/Scale/RotOrigin), pen freehand, selection box tracking
- `InputHandler::mouseButtonCallback(window, button, action, mods)` — all tool dispatch:
  - Select (click-to-select + selection box drag-and-release)
  - Draw Polygon/Line/Pen (vertex placement, right-click finish, grid snapping)
  - Fill (hit-test + enclosed region generation)
  - Move/Rotate/RotOrigin/Scale (hit-test, pushUndo, drag start/release)
  - Shape placement (grid snapping, generate* call, pushUndo)
- `InputHandler::scrollCallback(window, xoffset, yoffset)` — zoom in/out
- `InputHandler::keyCallback(window, key, scancode, action, mods)` — tool hotkeys (1–5, M, R, O, S), Ctrl+Z/Y/S/O, Delete/Backspace, Esc, Enter (finish line drawing)
- `InputHandler::framebufferSizeCallback(window, width, height)` — viewport resize
- `InputHandler::windowFocusCallback(window, focused)` — ImGui forwarding
- `InputHandler::cursorEnterCallback(window, entered)` — ImGui forwarding

### Hit-Testing (Clipping.cpp)
- `Clipping::isPointInPolygon(point, polygon)` — even-odd ray casting

### Entry Point (main.cpp)
- `main()` — engine instantiation, initialization, coordinate setup, run loop

---

## Member 4 — Engine State, Rendering, Serialization & Undo/Redo

**234226K — Bovinda Isanka**

**Files:** `Engine.cpp`, `Renderer.cpp`, `Engine.h`
**Category:** Core engine lifecycle, OpenGL draw calls, file I/O, undo stacks

### Engine Lifecycle (Engine.cpp / Engine.h)
- `Engine::Engine(width, height, title)` — constructor, state initialisation (includes `panelWidth = 195`, `currentFilePath{0}`)
- `Engine::~Engine()` — destructor, calls shutdown
- `Engine::initialize()` — GLFW/GLAD/ImGui initialisation, callback registration
- `Engine::run()` — main loop (ImGui frames, update, render, UI, swap/poll)
- `Engine::shutdown()` — ImGui/GLFW teardown
- `Engine::setWorldWindow(minX, maxX, minY, maxY)` — coordinate system
- `Engine::setViewport(width, height)` — OpenGL viewport + store dimensions
- `Engine::update(deltaTime)` — per-frame placeholder

### Undo/Redo (Engine.cpp)
- `Engine::pushUndo()` — snapshot shapes to undo stack, clear redo stack
- `Engine::undo()` — restore previous shape state
- `Engine::redo()` — restore next shape state

### Serialization & File Dialogs (Engine.cpp)
- `Engine::saveShapes(filename)` — write `.cad` text format
- `Engine::loadShapes(filename)` — read `.cad` text format
- `Engine::saveWithDialog()` — native Windows Save As dialog, stores path in `currentFilePath`
- `Engine::openWithDialog()` — native Windows Open dialog, stores path in `currentFilePath`

### Rendering (Renderer.cpp)
- `Renderer::Renderer(engine)` — constructor
- `Renderer::render()` — full frame: viewport offset for sidebar, clear, projection, grid, shape loop (clip + fill + stroke + selection highlight + in-progress preview + selection box)
- `Renderer::drawGrid()` — 10-unit grid lines
- `Renderer::scanLineFill(polygon)` — scan-line polygon fill

### State Declarations (Engine.h)
- All private member fields (shapes, selectedShape, tools, colors, drag state, undo/redo stacks, selection box, snap flag, `panelWidth`, `currentFilePath[260]`)
- Public API declarations (includes `saveWithDialog`, `openWithDialog`)
- Tool and ShapeType enums

---

## Member 5 — Line Clipping, Coordinate Mapping & UI

**234224D — Pamindu Piyumaka**

**Files:** `UIBuilder.cpp`, `Clipping.cpp`, `Renderer.cpp`
**Category:** Line clipping algorithms, viewport coordinate mapping, user interface

### Line Clipping (Clipping.cpp)
- `Clipping::computeRegionCode(x, y, box)` — 4-bit outcodes for Cohen-Sutherland
- `Clipping::cohenSutherlandClipLine(p1, p2, box)` — region-code line clipping
- `Clipping::liangBarskyClipLine(p1, p2, box)` — parametric line clipping

### Coordinate Mapping (Renderer.cpp)
- `Renderer::screenToWorld(screenX, screenY, worldX, worldY)` — viewport-to-world coordinate transform

### UI Layout (UIBuilder.cpp)
- `UIBuilder::UIBuilder(engine)` — constructor
- `UIBuilder::buildUI()` — complete UI layout:

  **Menu bar:**
  - File menu: Save (Ctrl+S, opens native dialog, auto-saves to last path after first time), Open (Ctrl+O, native dialog), Clear All, Exit
  - Edit menu: Undo (Ctrl+Z), Redo (Ctrl+Y)
  - View menu: Grid toggle, Snap to Grid toggle, Reset Zoom
  - Help menu: shortcut reference text, tool usage descriptions

  **Tool panel (left sidebar):**
  - Draw section: Select, Polygon, Line, Pen buttons (active highlighting)
  - Edit section: Fill button
  - Transform section: Move, Rotate, Rotate Origin, Scale buttons (disabled when no selection)
  - Shapes section: shape type combo (Rectangle/Circle/Triangle/Pentagon/Hexagon/Star), Size slider, Place button
  - Colors section: Fill and Stroke color editors (edits selected shape or defaults)
  - Selection section: vertex count, Mirror H, Mirror V, Delete button

  **Status bar (bottom):**
  - Current tool label, cursor world coordinates, shape count, drag info (scale factor during Scale, angle during Rotate)
