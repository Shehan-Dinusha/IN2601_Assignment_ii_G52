#ifndef CLIPPING_H
#define CLIPPING_H

#include "Transformations.h" // Includes Point2D
#include <vector>

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

    // Clips a polygon against a rectangular bounding box using Sutherland-Hodgman algorithm.
    // Modifies polygon in-place. Returns false if fully clipped away.
    static bool sutherlandHodgmanClipPolygon(std::vector<Point2D>& polygon, const BoundingBox& box);

    // Even-odd rule ray casting point-in-polygon test.
    // Returns true if point is inside the polygon.
    static bool isPointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon);

    // Finds the enclosed region (face) containing a click point,
    // formed by intersecting line segments from all shapes.
    // Fills 'out' with boundary vertices. Returns true if found.
    static bool findEnclosedRegion(std::vector<Point2D>& out, const Point2D& click,
        const std::vector<std::vector<Point2D>>& shapeVertices,
        const std::vector<bool>& shapeClosed);

private:
    // Helper function for region code calculation
    static int computeRegionCode(float x, float y, const BoundingBox& box);
};

#endif // CLIPPING_H
