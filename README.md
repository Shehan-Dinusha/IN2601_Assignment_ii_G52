# Computer Graphics Assignment II (IN2601)

This repository contains the OpenGL project for the IN2601 Computer Graphics Assignment II. It is pre-configured to build and run easily within Visual Studio Code.

## Prerequisites

To build and run this project, you need the following installed on your system:

- **Visual Studio Code** with the **C/C++ Extension** installed.
- **MinGW-w64** (or any other GCC compiler) installed.
- Ensure that the path to your compiler's `bin` directory (e.g., `C:\msys64\mingw64\bin` or `C:\mingw\bin`) is added to your Windows `PATH` environment variable.

## Setup Instructions

1. Clone this repository to your local machine.
2. Open the cloned folder in Visual Studio Code.
3. The `.vscode` folder is already included, which contains the build configurations.

## How to Build

We have configured a VS Code build task for this project.

1. In Visual Studio Code, press **Ctrl+Shift+B** (or navigate to **Terminal > Run Build Task**).
2. This will compile the project and generate an executable named `app.exe` in the root directory.

_Note: If building fails because `g++` is not recognized, please ensure your system `PATH` is set up correctly, or edit the `"command"` path in `.vscode/tasks.json` to point directly to your `g++.exe`._

## How to Run

After a successful build, you can run the application directly from the VS Code terminal:

```bash
./app.exe
```

## Folder Structure

- `src/` - Contains all `.cpp` and source files.
- `include/` - Contains required headers (GLAD, GLFW).
- `lib/` - Contains pre-compiled libraries used for linking.
- `.vscode/` - VS Code specific configuration for building and IntelliSense.
