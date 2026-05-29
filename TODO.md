# 2D Vector CAD Editor - Team Task Breakdown

This document outlines the division of labor for the 5 team members. The project infrastructure has been set up with clean separation of concerns. Inside the source files, you will find `// TODO:` comments pointing exactly to where you need to implement your logic.

---

## 🧑‍💻 Member 1: Core Math & Matrix Algebra

**Responsibility:** Implement the foundational linear algebra needed for 2D graphics using homogeneous coordinates.
**Target File:** `src/Transformations.cpp`

**Tasks:**

- [ ] Implement `Matrix3x3::operator*(const Matrix3x3& other)`: Write the logic to multiply two 3x3 matrices.
- [ ] Implement `Matrix3x3::operator*(const Point2D& point)`: Write the logic to multiply a 3x3 matrix by a 3-element column vector (x, y, w).
- [ ] Implement `getTranslationMatrix(tx, ty)`: Return a constructed matrix for translation.
- [ ] Implement `getScalingMatrix(sx, sy)`: Return a constructed matrix for scaling.
- [ ] Implement `getRotationMatrix(angleRadians)`: Return a constructed matrix for rotation (use `std::cos` and `std::sin`).

---

## 🧑‍💻 Member 2: Applied Geometry & Shape Manipulations

**Responsibility:** Use the core math created by Member 1 to efficiently transform collections of geometry (shapes).
**Target File:** `src/Transformations.cpp`

**Tasks:**

- [ ] Implement `translateShape(std::vector<Point2D>& shape, tx, ty)`: Apply translation to all points.
- [ ] Implement `scaleShape(std::vector<Point2D>& shape, sx, sy)`: Apply scaling to all points.
- [ ] Implement `rotateShapeAroundOrigin(std::vector<Point2D>& shape, angleRadians)`: Apply basic rotation.
- [ ] Implement `rotateShapeAroundPivot(...)`: Combine matrices (Translate -> Rotate -> Translate Back) to rotate shapes around an arbitrary pivot point.

---

## 🧑‍💻 Member 3: Engine Input, Camera & Interactions

**Responsibility:** Handle user inputs (mouse and keyboard) mapping them to world actions like Panning, Zooming, and Selecting.
**Target File:** `src/Engine.cpp`

**Tasks:**

- [ ] Implement `cursorPositionCallback(...)`: Track mouse movement to handle canvas panning or drawing a live line preview.
- [ ] Implement `mouseButtonCallback(...)`: Detect mouse clicks to draw points, select shapes, or finish a polygon.
- [ ] Implement `scrollCallback(...)`: Modify the `worldMin`/`worldMax` variables to zoom in and out of the canvas space.
- [ ] Integrate user inputs with the engine state so Member 4 can render them.

---

## 🧑‍💻 Member 4: OpenGL Rendering & Application State

**Responsibility:** Manage the application's visual output, graphic states, and main update loops.
**Target File:** `src/Engine.cpp`

**Tasks:**

- [ ] Implement `render()`: Setup `glBegin(GL_LINES)`, `glBegin(GL_POLYGON)`, etc., to loop through the active shapes and draw them on screen.
- [ ] Map the "World Coordinates" to the "Viewport Coordinates" using `glOrtho` or manual projection matrix logic before drawing.
- [ ] Implement `update(deltaTime)`: Handle continuous logic such as blinking selections or movement animations over time.
- [ ] Create data structures to hold the user's current shapes/lines inside the engine.

---

## 🧑‍💻 Member 5: Clipping Algorithms

**Responsibility:** Write the algorithmic logic to crop lines and shapes that fall outside the current viewport or bounding box.
**Target File:** `src/Clipping.cpp`

**Tasks:**

- [ ] Implement `computeRegionCode(x, y, box)`: Calculate the 4-bit region code (INSIDE, LEFT, RIGHT, BOTTOM, TOP) for a given point.
- [ ] Implement `cohenSutherlandClipLine(...)`: Execute the Cohen-Sutherland line clipping algorithm, updating the passed points and returning visibility.
- [ ] Implement `liangBarskyClipLine(...)`: Execute the Liang-Barsky parametric line clipping algorithm, updating the passed points and returning visibility.
- [ ] Work with Member 4 to ensure rendering only draws clipped lines.
