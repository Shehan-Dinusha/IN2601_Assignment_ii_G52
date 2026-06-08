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
    (void)other;
    return Matrix3x3(); // placeholder — return identity
}

// ============================================================
// MEMBER 1 TODO: Implement Matrix3x3 * Point2D multiplication
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Point2D Matrix3x3::operator*(const Point2D& point) const {
    return point; // placeholder — return point unchanged
}

// ============================================================
// MEMBER 1 TODO: Implement getTranslationMatrix
// Returns: [1 0 tx; 0 1 ty; 0 0 1]
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Transformations::getTranslationMatrix(float tx, float ty) {
    (void)tx; (void)ty;
    return Matrix3x3(); // placeholder — return identity
}

// ============================================================
// MEMBER 1 TODO: Implement getScalingMatrix
// Returns: [sx 0 0; 0 sy 0; 0 0 1]
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Transformations::getScalingMatrix(float sx, float sy) {
    (void)sx; (void)sy;
    return Matrix3x3(); // placeholder — return identity
}

// ============================================================
// MEMBER 1 TODO: Implement getRotationMatrix
// Returns: [cos -sin 0; sin cos 0; 0 0 1] (counter-clockwise)
// Use std::cos and std::sin from <cmath>
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
Matrix3x3 Transformations::getRotationMatrix(float angleRadians) {
    (void)angleRadians;
    return Matrix3x3(); // placeholder — return identity
}

Matrix3x3 Transformations::getReflectionMatrix(bool mirrorX, bool mirrorY) {
    Matrix3x3 mat;
    if (mirrorX) mat.m[0][0] = -1.0f;
    if (mirrorY) mat.m[1][1] = -1.0f;
    return mat;
}

// ============================================================
// MEMBER 2 TODO: Implement translateShape
// Use getTranslationMatrix(tx, ty) and apply to all vertices
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void Transformations::translateShape(std::vector<Point2D>& shape, float tx, float ty) {
    (void)shape; (void)tx; (void)ty;
    // TODO
}

// ============================================================
// MEMBER 2 TODO: Implement scaleShape
// Use getScalingMatrix(sx, sy) and apply to all vertices
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void Transformations::scaleShape(std::vector<Point2D>& shape, float sx, float sy) {
    (void)shape; (void)sx; (void)sy;
    // TODO
}

// ============================================================
// MEMBER 2 TODO: Implement rotateShapeAroundOrigin
// Use getRotationMatrix(angleRadians) and apply to all vertices
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void Transformations::rotateShapeAroundOrigin(std::vector<Point2D>& shape, float angleRadians) {
    (void)shape; (void)angleRadians;
    // TODO
}

// ============================================================
// MEMBER 2 TODO: Implement rotateShapeAroundPivot
// Compose: translate to origin → rotate → translate back
// Matrix chain: t2 * r * t1 applied to each vertex
// See COMPLETE_PROJECT_REFERENCE.md for full implementation
// ============================================================
void Transformations::rotateShapeAroundPivot(std::vector<Point2D>& shape, const Point2D& pivot, float angleRadians) {
    (void)shape; (void)pivot; (void)angleRadians;
    // TODO
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
