#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include "Shape.h"
#include "Renderer.h"
#include "UIBuilder.h"

struct GLFWwindow;

class Engine {
    friend class Renderer;
    friend class UIBuilder;
    friend class InputHandler;

public:
    enum Tool {
        TOOL_SELECT,
        TOOL_DRAW_POLYGON,
        TOOL_DRAW_LINE,
        TOOL_PEN,
        TOOL_FILL,
        TOOL_MOVE,
        TOOL_ROTATE,
        TOOL_ROTATE_ORIGIN,
        TOOL_SCALE,
        TOOL_SHAPE
    };

    enum ShapeType {
        SHAPE_RECTANGLE,
        SHAPE_CIRCLE,
        SHAPE_TRIANGLE,
        SHAPE_PENTAGON,
        SHAPE_HEXAGON,
        SHAPE_STAR
    };

    Engine(int width, int height, const std::string& title);
    ~Engine();

    bool initialize();
    void run();
    void shutdown();

    void setWorldWindow(float minX, float maxX, float minY, float maxY);
    void setViewport(int width, int height);

    void pushUndo();
    void undo();
    void redo();
    void saveShapes(const char* filename);
    void loadShapes(const char* filename);
    void saveWithDialog();
    void openWithDialog();

private:
    GLFWwindow* window;
    int windowWidth;
    int windowHeight;
    std::string windowTitle;

    float worldMinX, worldMaxX;
    float worldMinY, worldMaxY;

    std::vector<Shape> shapes;
    int selectedShapeIndex;
    std::vector<Point2D> currentShape;

    Tool activeTool;
    ShapeType activeShapeType;
    float shapeSize;
    float fillColor[3];
    float strokeColor[3];
    bool showGrid;
    bool isDrawing;
    bool isPanning;
    bool isDragging;
    double lastMouseX, lastMouseY;
    float dragStartX, dragStartY;
    float dragScaleFactor;
    float dragAngle;

    std::vector<std::vector<Shape>> undoStack;
    std::vector<std::vector<Shape>> redoStack;

    bool isSelecting;
    float selBoxX1, selBoxY1, selBoxX2, selBoxY2;
    bool snapToGridEnabled;
    float panelWidth;
    char currentFilePath[260];

    Renderer renderer;
    UIBuilder uiBuilder;

    void update(float deltaTime);
};

#endif