#include "../include/Transformations.h"
#include <cmath>

Matrix3x3::Matrix3x3() {
    // Initialize as identity matrix
    for (int i = 0; i < 3; ++i) {
        for (int j = 0; j < 3; ++j) {
            m[i][j] = (i == j) ? 1.0f : 0.0f;
        }
    }
}

Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
    Matrix3x3 result;
    // TODO: Implement 3x3 matrix multiplication
    // std::memset to 0 then loop multiply
    return result;
}

Point2D Matrix3x3::operator*(const Point2D& point) const {
    Point2D result;
    result.x = 0; result.y = 0; result.w = 0;
    // TODO: Implement matrix-vector multiplication
    return result;
}

Matrix3x3 Transformations::getTranslationMatrix(float tx, float ty) {
    Matrix3x3 mat;
    // TODO: Setup translation matrix values directly
    return mat;
}

Matrix3x3 Transformations::getScalingMatrix(float sx, float sy) {
    Matrix3x3 mat;
    // TODO: Setup scaling matrix values
    return mat;
}

Matrix3x3 Transformations::getRotationMatrix(float angleRadians) {
    Matrix3x3 mat;
    // TODO: Setup rotation matrix using std::cos and std::sin
    return mat;
}

void Transformations::translateShape(std::vector<Point2D>& shape, float tx, float ty) {
    // TODO: Get translation matrix and apply to all points in shape
}

void Transformations::scaleShape(std::vector<Point2D>& shape, float sx, float sy) {
    // TODO: Get scaling matrix and apply to all points in shape
}

void Transformations::rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians) {
    // TODO: Get rotation matrix and apply to all points in shape
}

void Transformations::rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians) {
    // TODO: Define a composite matrix: Translate(-pivot) -> Rotate(angle) -> Translate(pivot)
    // TODO: Apply the composite matrix to all points in shape
}
