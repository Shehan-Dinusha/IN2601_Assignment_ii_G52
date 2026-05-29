#ifndef CLIPPING_H
#define CLIPPING_H

#include "Transformations.h" // Includes Point2D

struct BoundingBox {
    float xMin;
    float xMax;
    float yMin;
    float yMax;

    BoundingBox(float xMin, float xMax, float yMin, float yMax)
        : xMin(xMin), xMax(xMax), yMin(yMin), yMax(yMax) {}
};

class Clipping {
public:
    // Clips a line segment defined by p1 and p2 using Cohen-Sutherland algorithm.
    // Modifies p1 and p2 directly if clipped.
    // Returns true if any part of the line is visible, false if fully rejected.
    static bool cohenSutherlandClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box);

    // Clips a line segment defined by p1 and p2 using Liang-Barsky algorithm.
    // Modifies p1 and p2 directly if clipped.
    // Returns true if any part of the line is visible, false if fully rejected.
    static bool liangBarskyClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box);

private:
    // Helper function for region code calculation
    static int computeRegionCode(float x, float y, const BoundingBox& box);
};

#endif // CLIPPING_H
