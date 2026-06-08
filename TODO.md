# 2D Vector CAD Editor - Team Task Breakdown

This document outlines the division of labor for the 5 team members. The project infrastructure has been set up with clean separation of concerns. Inside the source files, you will find `// TODO:` comments pointing exactly to where you need to implement your logic.

**Build order dependency:** Member 1 → Member 2 → Members 3, 4, 5 (can be done in parallel once 1 & 2 are complete).

---

## 🧑‍💻 Member 1: Core Math, Matrix Algebra & Polygon Clipping

**Responsibility:** Implement the foundational linear algebra for 2D graphics using homogeneous coordinates, plus the Sutherland–Hodgman polygon clipping algorithm.
**Target Files:** `src/Transformations.cpp`, `src/Clipping.cpp`

**Tasks:**

### Matrix Math (Transformations.cpp)
- [ ] Implement `Matrix3x3::operator*(const Matrix3x3& other)`: Multiply two 3x3 matrices.
- [ ] Implement `Matrix3x3::operator*(const Point2D& point)`: Multiply a 3x3 matrix by a (x, y, w) column vector.
- [ ] Implement `getTranslationMatrix(tx, ty)`: Return a translation matrix.
- [ ] Implement `getScalingMatrix(sx, sy)`: Return a scaling matrix.
- [ ] Implement `getRotationMatrix(angleRadians)`: Return a rotation matrix (use `std::cos` and `std::sin`).

### Graphics Algorithm (Clipping.cpp)
- [ ] Implement `sutherlandHodgmanClipPolygon(polygon, box)`: Clip a polygon against a rectangular bounding box edge-by-edge (left → right → bottom → top). For each clip edge, iterate polygon edges, keep inside vertices, and compute edge intersections. Modify the polygon in-place. Return `false` if fully clipped away.

---

## 🧑‍💻 Member 2: Applied Geometry & Shape Manipulations

**Responsibility:** Use the core math created by Member 1 to efficiently transform collections of geometry (shapes).
**Target File:** `src/Transformations.cpp`

**Tasks:**

- [ ] Implement `translateShape(std::vector<Point2D>& shape, tx, ty)`: Apply translation to all points.
- [ ] Implement `scaleShape(std::vector<Point2D>& shape, sx, sy)`: Apply scaling to all points.
- [ ] Implement `rotateShapeAroundOrigin(std::vector<Point2D>& shape, angleRadians)`: Apply basic rotation.
- [ ] Implement `rotateShapeAroundPivot(shape, pivot, angleRadians)`: Combine matrices (Translate → Rotate → Translate Back) to rotate shapes around an arbitrary pivot point — demonstrates homogeneous matrix composition.

---

## 🧑‍💻 Member 3: Engine Input, Camera & Hit-Testing Algorithm

**Responsibility:** Handle user inputs (mouse and keyboard) mapping them to world actions like Panning, Zooming, and Selecting. Implement the ray-casting algorithm for shape selection.
**Target Files:** `src/Engine.cpp`, `src/Clipping.cpp`

**Tasks:**

### Input Callbacks (Engine.cpp)
- [ ] Implement `cursorPositionCallback(...)`: Track mouse movement to handle canvas panning or drawing a live line preview.
- [ ] Implement `mouseButtonCallback(...)`: Detect mouse clicks to draw points, select shapes, or finish a polygon. Use `isPointInPolygon` for hit-testing.
- [ ] Implement `scrollCallback(...)`: Modify the `worldMin`/`worldMax` variables to zoom in and out of the canvas space.
- [ ] Integrate user inputs with the engine state so Member 4 can render them.

### Graphics Algorithm (Clipping.cpp)
- [ ] Implement `isPointInPolygon(point, polygon)`: Even-odd rule ray casting — cast a horizontal ray rightward from the point, count edge crossings. Return `true` if odd (inside), `false` if even (outside). Used by the mouse callback for shape selection.

---

## 🧑‍💻 Member 4: OpenGL Rendering, State & Scan-Line Fill Algorithm

**Responsibility:** Manage the application's visual output, graphic states, and main update loops. Implement the scan-line polygon fill algorithm.
**Target File:** `src/Engine.cpp`

**Notes:**
- The GLFW profile hint in `Engine::initialize()` uses `GLFW_OPENGL_CORE_PROFILE`, which disables `glBegin`/`glEnd`. If your implementation relies on immediate mode, change it to `GLFW_OPENGL_COMPAT_PROFILE`.
- The scan-line fill should draw horizontal pixel segments using `glBegin(GL_LINES)` (or `GL_POINTS`) inside `render()`.

**Tasks:**

### Rendering & State (Engine.cpp)
- [ ] Create data structures (e.g., `std::vector<std::vector<Point2D>>`) to hold the user's current shapes/lines inside the engine.
- [ ] Map "World Coordinates" to "Viewport Coordinates" using `glOrtho` before drawing.
- [ ] Implement `render()`: Loop through active shapes and draw them on screen.
- [ ] Implement `update(deltaTime)`: Handle continuous logic such as blinking selections or movement animations over time.
- [ ] Coordinate data structure layout with Member 3.

### Graphics Algorithm (Engine.cpp)
- [ ] Implement `scanLineFill(polygon)`: Find `yMin`/`yMax`, for each scan line compute edge intersections, sort by x, and draw horizontal segments between even-odd pairs. Call from `render()` to fill polygons.

---

## 🧑‍💻 Member 5: Line Clipping Algorithms

**Responsibility:** Write the algorithmic logic to crop lines that fall outside the current viewport or bounding box.
**Target File:** `src/Clipping.cpp`

**Tasks:**

- [ ] Implement `computeRegionCode(x, y, box)`: Calculate the 4-bit region code (INSIDE, LEFT, RIGHT, BOTTOM, TOP) for a given point.
- [ ] Implement `cohenSutherlandClipLine(p1, p2, box)`: Execute the Cohen-Sutherland line clipping algorithm, updating the passed points and returning visibility.
- [ ] Implement `liangBarskyClipLine(p1, p2, box)`: Execute the Liang-Barsky parametric line clipping algorithm, updating the passed points and returning visibility.
- [ ] Work with Member 4 to ensure rendering only draws clipped lines.
