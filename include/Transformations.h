#ifndef TRANSFORMATIONS_H
#define TRANSFORMATIONS_H

#include <vector>

struct Point2D {
    float x;
    float y;
    float w; // Homogeneous coordinate, typically 1.0

    Point2D(float x = 0.0f, float y = 0.0f, float w = 1.0f) : x(x), y(y), w(w) {}
};

struct Matrix3x3 {
    float m[3][3];

    Matrix3x3();
    
    // Enable matrix and vector multiplication naturally
    Matrix3x3 operator*(const Matrix3x3& other) const;
    Point2D operator*(const Point2D& point) const;
};

class Transformations {
public:
    static Matrix3x3 getTranslationMatrix(float tx, float ty);
    static Matrix3x3 getScalingMatrix(float sx, float sy);
    static Matrix3x3 getRotationMatrix(float angleRadians);
    static Matrix3x3 getReflectionMatrix(bool mirrorX, bool mirrorY);

    static void translateShape(std::vector<Point2D>& shape, float tx, float ty);
    static void scaleShape(std::vector<Point2D>& shape, float sx, float sy);
    static void rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians);
    static void rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians);
    static void mirrorShape(std::vector<Point2D>& shape, bool horizontal, bool vertical);

    static void generateRectangle(std::vector<Point2D>& out, float cx, float cy, float w, float h);
    static void generateCircle(std::vector<Point2D>& out, float cx, float cy, float radius, int segments = 32);
    static void generateTriangle(std::vector<Point2D>& out, float cx, float cy, float size);
    static void generateRegularPolygon(std::vector<Point2D>& out, float cx, float cy, float radius, int sides);
    static void generateStar(std::vector<Point2D>& out, float cx, float cy, float outerRadius, float innerRadius = 0.4f);
};

#endif // TRANSFORMATIONS_H
