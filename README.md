# Computer Graphics Assignment II (IN2601)

This repository contains the OpenGL project for the IN2601 Computer Graphics Assignment II.

## Prerequisites

### Option A — Docker (recommended, no toolchain install)

- **Docker Desktop** (free, [download here](https://www.docker.com/products/docker-desktop/))

### Option B — Native build (alternative)

- **Visual Studio Code** with the **C/C++ Extension** installed.
- **MinGW-w64** (or any other GCC compiler) installed and on your `PATH`.

## Setup

1. Clone this repository to your local machine.
2. Open the cloned folder in Visual Studio Code or a terminal.

## How to Build

### Docker (recommended)

```cmd
:: Windows cmd.exe:
build.cmd

:: Git Bash / WSL:
./build.sh
```

First run downloads the cross-compiler image (~400 MB). Subsequent runs are instant. `app.exe` appears in the project root.

### VS Code native build (alternative)

Press **Ctrl+Shift+B** (or **Terminal > Run Build Task**). Requires **MinGW-w64 g++** on `PATH`.

> If exit code -1 appears without g++ errors, the project path likely contains **OneDrive** or **spaces** — move to a short flat path like `C:\IN2601_Assignment_ii` and retry.

## How to Run

After a successful build, run:

```bash
./app.exe
```

`glfw3.dll` is already in the repo — no manual copy needed.

## Folder Structure

- `src/` — All `.cpp` and source files
- `include/` — Project headers
- `lib/` — Pre-compiled libraries for linking
- `vendor/` — Third-party code (glad, glfw, imgui — do not modify)
- `.vscode/` — VS Code build configuration
