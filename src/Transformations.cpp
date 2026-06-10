#include "Transformations.h"
#include <cmath>

Matrix3x3::Matrix3x3() {
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
            m[i][j] = (i == j) ? 1.0f : 0.0f;
}

// ============================================================
// MEMBER 1 TODO: Implement Matrix3x3 matrix multiplication
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Matrix3x3::operator*(const Matrix3x3& other) const {
    Matrix3x3 result;
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j) {
            result.m[i][j] = 0.0f;
            for (int k = 0; k < 3; ++k)
                result.m[i][j] += m[i][k] * other.m[k][j];
        }
    return result;
}

// ============================================================
// MEMBER 1 TODO: Implement Matrix3x3 * Point2D multiplication
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Point2D Matrix3x3::operator*(const Point2D& point) const {
    Point2D result;
    result.x = m[0][0] * point.x + m[0][1] * point.y + m[0][2] * point.w;
    result.y = m[1][0] * point.x + m[1][1] * point.y + m[1][2] * point.w;
    result.w = m[2][0] * point.x + m[2][1] * point.y + m[2][2] * point.w;
    return result;
}

// ============================================================
// MEMBER 1 TODO: Implement getTranslationMatrix
// Returns: [1 0 tx; 0 1 ty; 0 0 1]
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Transformations::getTranslationMatrix(float tx, float ty) {
    Matrix3x3 mat;
    mat.m[0][2] = tx;
    mat.m[1][2] = ty;
    return mat;
}

// ============================================================
// MEMBER 1 TODO: Implement getScalingMatrix
// Returns: [sx 0 0; 0 sy 0; 0 0 1]
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Transformations::getScalingMatrix(float sx, float sy) {
    Matrix3x3 mat;
    mat.m[0][0] = sx;
    mat.m[1][1] = sy;
    return mat;
}

// ============================================================
// MEMBER 1 TODO: Implement getRotationMatrix
// Returns: [cos -sin 0; sin cos 0; 0 0 1] (counter-clockwise)
// Use std::cos and std::sin from <cmath>
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Transformations::getRotationMatrix(float angleRadians) {
    Matrix3x3 mat;
    float c = std::cos(angleRadians), s = std::sin(angleRadians);
    mat.m[0][0] = c; mat.m[0][1] = -s;
    mat.m[1][0] = s; mat.m[1][1] =  c;
    return mat;
}

Matrix3x3 Transformations::getReflectionMatrix(bool mirrorX, bool mirrorY) {
    Matrix3x3 mat;
    if (mirrorX) mat.m[0][0] = -1.0f;
    if (mirrorY) mat.m[1][1] = -1.0f;
    return mat;
}

// ============================================================
// MEMBER 2: Implement translateShape
// Apply translation matrix to every vertex of the shape
// ============================================================
void Transformations::translateShape(std::vector<Point2D>& shape, float tx, float ty) {
    Matrix3x3 translationMatrix = getTranslationMatrix(tx, ty);

    for (auto& point : shape) {
        point = translationMatrix * point;
    }
}

// ============================================================
// MEMBER 2: Implement scaleShape
// Apply scaling matrix to every vertex of the shape
// ============================================================
void Transformations::scaleShape(std::vector<Point2D>& shape, float sx, float sy) {
    Matrix3x3 scalingMatrix = getScalingMatrix(sx, sy);

    for (auto& point : shape) {
        point = scalingMatrix * point;
    }
}

// ============================================================
// MEMBER 2: Implement rotateShapeAroundOrigin
// Apply rotation matrix to every vertex of the shape
// ============================================================
void Transformations::rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians) {
    Matrix3x3 rotationMatrix = getRotationMatrix(angleRadians);

    for (auto& point : shape) {
        point = rotationMatrix * point;
    }
}

// ============================================================
// MEMBER 2: Implement rotateShapeAroundPivot
// Rotate shape around an arbitrary pivot point
//
// Steps:
// 1. Move pivot point to origin
// 2. Rotate around origin
// 3. Move pivot point back to original position
//
// Composite matrix = Translate Back * Rotate * Translate To Origin
// ============================================================
void Transformations::rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians) {
    Matrix3x3 translateToOrigin = getTranslationMatrix(-pivot.x, -pivot.y);
    Matrix3x3 rotationMatrix = getRotationMatrix(angleRadians);
    Matrix3x3 translateBack = getTranslationMatrix(pivot.x, pivot.y);

    Matrix3x3 compositeMatrix = translateBack * rotationMatrix * translateToOrigin;

    for (auto& point : shape) {
        point = compositeMatrix * point;
    }
}

void Transformations::mirrorShape(std::vector<Point2D>& shape, bool horizontal, bool vertical) {
    float cx = 0, cy = 0;
    for (auto& pt : shape) { cx += pt.x; cy += pt.y; }
    cx /= shape.size(); cy /= shape.size();

    Matrix3x3 t1 = getTranslationMatrix(-cx, -cy);
    Matrix3x3 r = getReflectionMatrix(horizontal, vertical);
    Matrix3x3 t2 = getTranslationMatrix(cx, cy);
    Matrix3x3 comp = t2 * r * t1;
    for (auto& pt : shape) pt = comp * pt;
}

void Transformations::generateRectangle(std::vector<Point2D>& out, float cx, float cy, float w, float h) {
    float hw = w / 2, hh = h / 2;
    out.push_back(Point2D(cx - hw, cy - hh));
    out.push_back(Point2D(cx + hw, cy - hh));
    out.push_back(Point2D(cx + hw, cy + hh));
    out.push_back(Point2D(cx - hw, cy + hh));
}

void Transformations::generateCircle(std::vector<Point2D>& out, float cx, float cy, float radius, int segments) {
    for (int i = 0; i < segments; ++i) {
        float a = 6.2831853f * i / segments;
        out.push_back(Point2D(cx + radius * std::cos(a), cy + radius * std::sin(a)));
    }
}

void Transformations::generateTriangle(std::vector<Point2D>& out, float cx, float cy, float size) {
    float r = size / 2;
    out.push_back(Point2D(cx, cy + r));
    out.push_back(Point2D(cx - 0.866f * r, cy - 0.5f * r));
    out.push_back(Point2D(cx + 0.866f * r, cy - 0.5f * r));
}

void Transformations::generateRegularPolygon(std::vector<Point2D>& out, float cx, float cy, float radius, int sides) {
    for (int i = 0; i < sides; ++i) {
        float a = 6.2831853f * i / sides - 1.5707963f;
        out.push_back(Point2D(cx + radius * std::cos(a), cy + radius * std::sin(a)));
    }
}

void Transformations::generateStar(std::vector<Point2D>& out, float cx, float cy, float outerRadius, float innerRadius) {
    for (int i = 0; i < 5; ++i) {
        float a = 6.2831853f * i / 5 - 1.5707963f;
        float a2 = a + 3.1415926f / 5;
        out.push_back(Point2D(cx + outerRadius * std::cos(a), cy + outerRadius * std::sin(a)));
        out.push_back(Point2D(cx + innerRadius * std::cos(a2), cy + innerRadius * std::sin(a2)));
    }
}