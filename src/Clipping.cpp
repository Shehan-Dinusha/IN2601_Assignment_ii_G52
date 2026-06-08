#include "Clipping.h"
#include <cmath>
#include <algorithm>
#include <limits>

// Define region codes for Cohen-Sutherland
const int INSIDE = 0; // 0000
const int LEFT   = 1; // 0001
const int RIGHT  = 2; // 0010
const int BOTTOM = 4; // 0100
const int TOP    = 8; // 1000

static float cross2D(const Point2D& a, const Point2D& b) {
    return a.x * b.y - a.y * b.x;
}

static float dot2D(const Point2D& a, const Point2D& b) {
    return a.x * b.x + a.y * b.y;
}

static Point2D sub2D(const Point2D& a, const Point2D& b) {
    return Point2D(a.x - b.x, a.y - b.y, 1.0f);
}

// Segment intersection test. Returns true and fills 'out' if segments (a,b) and (c,d) intersect.
static bool segSegIntersect(const Point2D& a, const Point2D& b,
                             const Point2D& c, const Point2D& d,
                             Point2D& out) {
    Point2D ab = sub2D(b, a);
    Point2D cd = sub2D(d, c);
    float denom = cross2D(ab, cd);
    if (fabs(denom) < 1e-8f) return false;
    Point2D ac = sub2D(c, a);
    float t = cross2D(ac, cd) / denom;
    float u = cross2D(ac, ab) / denom;
    if (t >= 0.0f && t <= 1.0f && u >= 0.0f && u <= 1.0f) {
        out.x = a.x + t * ab.x;
        out.y = a.y + t * ab.y;
        out.w = 1.0f;
        return true;
    }
    return false;
}

// Squared distance between two points
static float distSq(const Point2D& a, const Point2D& b) {
    float dx = a.x - b.x, dy = a.y - b.y;
    return dx * dx + dy * dy;
}

// Normalize angle to [0, 2π)
static float normAngle(float a) {
    const float twoPi = 6.28318530718f;
    while (a < 0.0f) a += twoPi;
    while (a >= twoPi) a -= twoPi;
    return a;
}

bool Clipping::findEnclosedRegion(std::vector<Point2D>& out, const Point2D& click,
    const std::vector<std::vector<Point2D>>& shapeVertices,
    const std::vector<bool>& shapeClosed) {

    const float EPS = 1e-5f;
    out.clear();

    // ---- Step 1: Collect all segments ----
    struct RawSeg { Point2D a, b; };
    std::vector<RawSeg> rawSegs;
    for (size_t si = 0; si < shapeVertices.size(); ++si) {
        const auto& verts = shapeVertices[si];
        if (verts.size() < 2) continue;
        for (size_t i = 0; i + 1 < verts.size(); ++i)
            rawSegs.push_back({verts[i], verts[i + 1]});
        if (shapeClosed[si] && verts.size() >= 3)
            rawSegs.push_back({verts.back(), verts.front()});
    }
    if (rawSegs.empty()) return false;

    // ---- Step 2: Build vertex list + edge list, splitting at intersections ----
    std::vector<Point2D> vtx;
    std::vector<std::pair<int,int>> edges;

    auto addVtx = [&](const Point2D& p) -> int {
        for (int i = 0; i < (int)vtx.size(); ++i)
            if (distSq(vtx[i], p) < EPS * EPS)
                return i;
        vtx.push_back(p);
        return (int)vtx.size() - 1;
    };

    for (auto& rs : rawSegs) {
        int ai = addVtx(rs.a);
        int bi = addVtx(rs.b);
        edges.push_back({ai, bi});
    }

    bool splitOccurred = true;
    while (splitOccurred) {
        splitOccurred = false;
        size_t eCount = edges.size();
        for (size_t i = 0; i < eCount; ++i) {
            int a1 = edges[i].first, b1 = edges[i].second;
            for (size_t j = i + 1; j < eCount; ++j) {
                int a2 = edges[j].first, b2 = edges[j].second;
                if (a1 == a2 || a1 == b2 || b1 == a2 || b1 == b2) continue;

                Point2D inter;
                if (segSegIntersect(vtx[a1], vtx[b1], vtx[a2], vtx[b2], inter)) {
                    bool atEnd1 = distSq(inter, vtx[a1]) < EPS*EPS ||
                                  distSq(inter, vtx[b1]) < EPS*EPS;
                    bool atEnd2 = distSq(inter, vtx[a2]) < EPS*EPS ||
                                  distSq(inter, vtx[b2]) < EPS*EPS;
                    if (atEnd1 || atEnd2) continue;

                    int vi = addVtx(inter);
                    edges[i] = {a1, vi};
                    edges.push_back({vi, b1});
                    edges[j] = {a2, vi};
                    edges.push_back({vi, b2});
                    eCount = edges.size();
                    splitOccurred = true;
                }
            }
        }
    }

    // ---- Step 3: Build adjacency and sort neighbors by angle at each vertex ----
    std::vector<std::vector<int>> adj(vtx.size());
    for (auto& e : edges) {
        adj[e.first].push_back(e.second);
        adj[e.second].push_back(e.first);
    }
    for (int i = 0; i < (int)adj.size(); ++i) {
        std::sort(adj[i].begin(), adj[i].end(), [&](int a, int b) {
            float ax = vtx[a].x - vtx[i].x, ay = vtx[a].y - vtx[i].y;
            float bx = vtx[b].x - vtx[i].x, by = vtx[b].y - vtx[i].y;
            return normAngle(atan2f(ay, ax)) < normAngle(atan2f(by, bx));
        });
    }

    // ---- Step 4: Cast ray rightward from click, find nearest hit ----
    struct Hit { int vi; int ni; };
    Hit bestHit = {-1, -1};
    float bestDist = std::numeric_limits<float>::max();

    for (int ei = 0; ei < (int)edges.size(); ++ei) {
        int ai = edges[ei].first, bi = edges[ei].second;
        float y1 = vtx[ai].y, y2 = vtx[bi].y;
        float ymin = std::min(y1, y2), ymax = std::max(y1, y2);
        if (ymin >= click.y || ymax <= click.y) continue;
        if (fabs(y2 - y1) < EPS) continue;
        float x_at_y = vtx[ai].x + (click.y - vtx[ai].y) * (vtx[bi].x - vtx[ai].x) / (vtx[bi].y - vtx[ai].y);
        if (x_at_y <= click.x + EPS) continue;
        float dist = x_at_y - click.x;
        if (dist < bestDist) {
            bestDist = dist;
            float d1 = distSq(vtx[ai], Point2D(x_at_y, click.y));
            float d2 = distSq(vtx[bi], Point2D(x_at_y, click.y));
            bestHit = (d1 <= d2) ? Hit{ai, bi} : Hit{bi, ai};
        }
    }
    if (bestHit.vi < 0) return false;

    // ---- Step 5: Walk the face boundary ----
    std::vector<int> boundary;
    std::vector<std::vector<bool>> edgeUsed(vtx.size());
    for (int i = 0; i < (int)vtx.size(); ++i)
        edgeUsed[i].resize(adj[i].size(), false);

    int cur = bestHit.vi;
    int prev = bestHit.ni;
    int start = cur;

    auto findNextNeighbor = [&](int cv, int pv) -> int {
        auto& nbrs = adj[cv];
        int pos = -1;
        for (int k = 0; k < (int)nbrs.size(); ++k) {
            if (nbrs[k] == pv) { pos = k; break; }
        }
        if (pos < 0) return -1;
        // Next neighbor counterclockwise from pv
        int next = (pos + 1) % (int)nbrs.size();
        if (edgeUsed[cv][next]) return -1;
        edgeUsed[cv][next] = true;
        int nv = nbrs[next];
        for (int k = 0; k < (int)adj[nv].size(); ++k)
            if (adj[nv][k] == cv) { edgeUsed[nv][k] = true; break; }
        return nv;
    };

    for (int iter = 0; iter < 10000; ++iter) {
        boundary.push_back(cur);
        if (cur == start && iter > 2) {
            boundary.pop_back();
            break;
        }
        int nxt = findNextNeighbor(cur, prev);
        if (nxt < 0) break;
        prev = cur;
        cur = nxt;
    }
    if (boundary.size() < 3) return false;
    std::vector<Point2D> poly;
    for (int idx : boundary) poly.push_back(vtx[idx]);
    if (!isPointInPolygon(click, poly)) return false;

    out = std::move(poly);
    return true;
}

int Clipping::computeRegionCode(float x, float y, const BoundingBox& box) {
    int code = INSIDE;
    return code;
}

bool Clipping::cohenSutherlandClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    return true;
}

bool Clipping::liangBarskyClipLine(Point2D& p1, Point2D& p2, const BoundingBox& box) {
    return true;
}

bool Clipping::sutherlandHodgmanClipPolygon(std::vector<Point2D>& polygon, const BoundingBox& box) {
    return true;
}

bool Clipping::isPointInPolygon(const Point2D& point, const std::vector<Point2D>& polygon) {
    int crossings = 0;
    size_t n = polygon.size();
    for (size_t i = 0; i < n; ++i) {
        float xi = polygon[i].x, yi = polygon[i].y;
        float xj = polygon[(i + 1) % n].x, yj = polygon[(i + 1) % n].y;
        if ((yi > point.y) != (yj > point.y)) {
            float x_intersect = (xj - xi) * (point.y - yi) / (yj - yi) + xi;
            if (point.x < x_intersect)
                crossings++;
        }
    }
    return (crossings % 2) == 1;
}
