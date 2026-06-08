#include "Shape.h"
#include "Clipping.h"

Point2D Shape::centroid() const {
    float cx = 0.0f, cy = 0.0f;
    for (auto& pt : vertices) {
        cx += pt.x;
        cy += pt.y;
    }
    size_t n = vertices.size();
    return Point2D(n ? cx / n : 0.0f, n ? cy / n : 0.0f);
}

bool Shape::contains(const Point2D& point) const {
    return Clipping::isPointInPolygon(point, vertices);
}

void Shape::translate(float dx, float dy) {
    Transformations::translateShape(vertices, dx, dy);
}

void Shape::rotate(float angleRadians, const Point2D& pivot) {
    Transformations::rotateShapeAroundPivot(vertices, pivot, angleRadians);
}

void Shape::mirror(bool horizontal, bool vertical) {
    Transformations::mirrorShape(vertices, horizontal, vertical);
}
