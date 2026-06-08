#ifndef SHAPE_H
#define SHAPE_H

#include "Transformations.h"
#include <vector>

class Shape {
public:
    std::vector<Point2D> vertices;
    float fill[3] = {0.0f, 0.0f, 0.0f};
    float stroke[3] = {1.0f, 1.0f, 1.0f};
    bool closed = true;

    Point2D centroid() const;
    bool contains(const Point2D& point) const;
    void translate(float dx, float dy);
    void rotate(float angleRadians, const Point2D& pivot);
    void mirror(bool horizontal, bool vertical);
};

#endif
