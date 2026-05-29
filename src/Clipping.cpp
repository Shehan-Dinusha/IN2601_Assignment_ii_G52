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
