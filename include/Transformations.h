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
    // Generate transformation matrices
    static Matrix3x3 getTranslationMatrix(float tx, float ty);
    static Matrix3x3 getScalingMatrix(float sx, float sy);
    static Matrix3x3 getRotationMatrix(float angleRadians);

    // Convenience functions to apply transformations to a list of points
    static void translateShape(std::vector<Point2D>& shape, float tx, float ty);
    static void scaleShape(std::vector<Point2D>& shape, float sx, float sy);
    static void rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians);
    
    // Composite transformation
    static void rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians);
};

#endif // TRANSFORMATIONS_H
