# 2D Vector CAD Editor — IN2601 Computer Graphics Assignment II

An interactive 2D vector graphics editor built with C++17, OpenGL (immediate mode), GLFW, and Dear ImGui. Users can draw, select, transform, fill, and manage vector shapes on a zoomable/pannable canvas with grid snapping and full undo/redo support.

## Features

| Category | What you can do |
|----------|----------------|
| **Draw** | Polygon (click vertices, right-click finish, max 100 vertices), Line (click vertices, right-click undo last point, Enter = finish), Pen (freehand click-drag), 6 predefined shapes (Rectangle, Circle, Triangle, Pentagon, Hexagon, Star) |
| **Select** | Click to select a shape (highlights gold), or drag a selection box to add all shapes whose centroid falls inside (multi-select) |
| **Transform** | Move, Rotate (about centroid), Rotate around Origin, Uniform Scale — applies to all selected shapes as a group via click-drag on canvas |
| **Fill / Stroke** | Fill tool click to recolor a shape or create an enclosed region from intersecting segments (multi-directional ray); color editors in the UI panel |
| **Z-Order** | Bring to Front / Send to Back buttons in Selection panel (affects all selected shapes) |
| **Copy / Paste** | Ctrl+C copies all selected shapes, Ctrl+V pastes with 10-unit offset |
| **Mirror / Delete** | Mirror Horizontal / Vertical about centroid from UI; Delete key or Delete button — all apply to all selected shapes |
| **Undo / Redo** | Ctrl+Z / Ctrl+Y, 100-level undo/redo stacks (snapshots of entire shape state with depth truncation) |
| **Save / Load / Export** | `.cad` text format via File → Save (Ctrl+S) / Open (Ctrl+O); SVG export via File → Export SVG |
| **Vertex Snapping** | Drawing tools auto-snap to nearest existing vertex/edge within 5 units (plus optional grid snap) |
| **Grid** | Toggleable 10-unit grid; Snap to Grid snaps vertex placement to nearest 10-unit multiple |
| **Pan / Zoom** | Middle-mouse drag to pan, scroll to zoom in/out; zoom percentage shown in status bar |
| **Workspace** | Clear All, Reset Zoom; on-canvas drag readout during transforms |

## Quick Start

### Prerequisites

- **MinGW-w64** (GCC with C++17 support) — add its `bin` directory (e.g. `C:\msys64\ucrt64\bin`) to your system `PATH` so `g++` is accessible from any terminal
- **Visual Studio Code** (optional, build task preconfigured)

### Build

**VS Code:** Press **Ctrl+Shift+B** (uses `.vscode/tasks.json`).

**Command line:**
```bash
g++ -g -std=c++17 -Iinclude -Ivendor/glad -Ivendor/glfw -Ivendor/imgui -Llib src/*.cpp vendor/glad/glad.c vendor/imgui/*.cpp -lglfw3dll -lcomdlg32 -lopengl32 -lglu32 -o app.exe
```

> If `g++` is not recognized, add your MinGW `bin` directory to `PATH` or update the compiler path in `.vscode/tasks.json`.

### Run

```bash
app.exe
```

> **Important:** `glfw3.dll` must be in the same directory as `app.exe`. It is already at the project root and in `lib/`. Close `app.exe` before rebuilding (otherwise you'll get a "Permission denied" error).

## Controls

| Key | Action |
|-----|--------|
| `1` | Select tool |
| `2` | Polygon tool |
| `3` | Line tool |
| `4` | Pen tool |
| `5` | Fill tool |
| `M` | Move tool |
| `R` | Rotate tool (about centroid) |
| `O` | Rotate around Origin tool |
| `S` | Scale tool |
| `Ctrl+Z / Ctrl+Y` | Undo / Redo |
| `Ctrl+S / Ctrl+O` | Save / Open `.cad` file |
| `Ctrl+C / Ctrl+V` | Copy / Paste selected shape |
| `Del / Backspace` | Delete selected shape(s) |
| `Enter` | Finish line drawing (save current line as shape) |
| `Esc` | Cancel drawing (or exit if nothing drawing) |
| `Right-click` | Undo last point (Line tool) / Finish polygon (Polygon tool) |
| **Middle-drag** | Pan canvas |
| **Scroll** | Zoom in/out |

## Project Structure

```
├── src/              Source files (.cpp)
│   ├── main.cpp
│   ├── Engine.cpp         Core state machine & lifecycle
│   ├── Renderer.cpp       OpenGL draw calls (stencil-buffer flood fill, grid, selection box)
│   ├── InputHandler.cpp   GLFW callbacks (mouse, keyboard, window)
│   ├── UIBuilder.cpp      Dear ImGui UI (menus, tool buttons, panels, status bar)
│   ├── Clipping.cpp       Algorithms: Sutherland-Hodgman, Liang-Barsky, point-in-polygon, enclosed region
│   ├── Transformations.cpp Matrix ops, shape transforms, shape generators
│   └── Shape.cpp          Shape convenience methods (delegates to Clipping/Transformations)
├── include/          Headers (.h)
├── vendor/           Third-party: GLAD, GLFW, Dear ImGui
├── lib/              Pre-compiled libraries (glfw3.dll, etc.)
├── .vscode/          VS Code build task config
├── AGENTS.md         AI agent reference (build, architecture, conventions)
└── MemberTasks.md    Task assignment per team member
```

## Architecture

| Class | Role |
|-------|------|
| `Engine` | Central state machine — owns all data (shapes, selection, tools, colors, undo stacks). Holds `Renderer` and `UIBuilder` instances. |
| `Renderer` | OpenGL immediate-mode drawing — projection, clipping, fill, stroke, grid, selection box. Reads Engine state. |
| `InputHandler` | Static GLFW callbacks that update Engine state. |
| `UIBuilder` | Dear ImGui panels and menus that read/write Engine state. |
| `Shape` | Vertex data + convenience methods (`centroid`, `contains`, `translate`, `rotate`, `mirror`). |
| `Clipping` | Static geometry algorithms. |
| `Transformations` | Static 3x3 homogeneous matrix ops and shape generators. |

## Member Task Division

See `MemberTasks.md` for a complete breakdown of which team member owns which function across all source files.
