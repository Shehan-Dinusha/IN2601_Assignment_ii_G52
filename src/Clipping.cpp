#include "../include/Clipping.h"

// Define region codes for Cohen-Sutherland
const int INSIDE = 0; // 0000
const int LEFT   = 1; // 0001
const int RIGHT  = 2; // 0010
const int BOTTOM = 4; // 0100
const int TOP    = 8; // 1000

int Clipping::computeRegionCode(float x, float y, const BoundingBox& box) {
    int code = INSIDE;

    // TODO: Calculate the region code based on x, y and the bounding box
    
    return code;
}

bool Clipping::cohenSutherlandClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    // TODO: Implement the Cohen-Sutherland line clipping algorithm
    // 1. Compute region codes for both endpoints
    // 2. Loop until trivially accepted or trivially rejected
    // 3. Find intersection with boundaries
    // 4. Update coordinates

    return true; // Placeholder, return true if visible
}

bool Clipping::liangBarskyClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    // TODO: Implement the Liang-Barsky line clipping algorithm
    // 1. Calculate p and q values for each boundary
    // 2. Find maximum of entry parameters and minimum of exit parameters
    // 3. Check for visibility based on these parameter max/mins
    // 4. Update coordinates using parametric equations

    return true; // Placeholder, return true if visible
}

bool Clipping::sutherlandHodgmanClipPolygon(std::vector<Point2D>& polygon, const BoundingBox& box) {
    // TODO: Implement the Sutherland-Hodgman polygon clipping algorithm (Member 1)
    // For each clip edge (left, right, bottom, top):
    //   1. Iterate polygon edges (current → next)
    //   2. For each edge, test if start and end are inside the current clip boundary
    //   3. Add appropriate points to output (inside→inside: add end; inside→outside: add intersection;
    //      outside→inside: add intersection then end; outside→outside: add nothing)
    //   4. Replace polygon with output for next clip edge
    // Return false if polygon is empty after clipping

    return true; // Placeholder
}

bool Clipping::isPointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon) {
    // TODO: Implement ray-casting point-in-polygon test (Member 3)
    // 1. For each edge of the polygon, check if a horizontal ray to the right from the point crosses it
    // 2. Count crossings (odd = inside, even = outside)
    // Edge crosses the ray if: (edge.p1.y > point.y) != (edge.p2.y > point.y)
    //   AND point.x < (edge.p2.x - edge.p1.x) * (point.y - edge.p1.y) / (edge.p2.y - edge.p1.y) + edge.p1.x

    return false; // Placeholder
}
