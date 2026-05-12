//
// Created by eliasw on 12.05.26.
//

#ifndef BACHELORPROJECT_BOUNDINGVOLUME_H
#define BACHELORPROJECT_BOUNDINGVOLUME_H
#include "hittable.h"
#include "triangleModel.h"
#include <algorithm>

enum axis {
    X, Y, Z
};

class boundingVolume : public hittable {
public:
    glm::vec3 minPoint{}, maxPoint{};
    int start = 0;
    int count = 0;
    bool isLeaf = true;
    axis longestAxis = X;

    boundingVolume() {
        minPoint = glm::vec3(1) * std::numeric_limits<float>::infinity();
        maxPoint = glm::vec3(-1) * std::numeric_limits<float>::infinity();
    }

    bool hit(const ray &r, interval ray_t, hitRecord &rec) const override {
        // AABB hit detection using slab test
        auto close = -std::numeric_limits<float>::infinity();
        auto far = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 3; i++) {
            const float rayDir = r.direction()[i];
            if (std::abs(rayDir) < 1e-6f) {
                if (r.origin()[i] < minPoint[i] || r.origin()[i] > maxPoint[i]) {
                    return false;
                }
                continue;
            }

            const auto t_low = (minPoint[i] - r.origin()[i]) / rayDir;
            const auto t_high = (maxPoint[i] - r.origin()[i]) / rayDir;
            const auto t_close_i = glm::min(t_low, t_high);
            const auto t_far_i = glm::max(t_low, t_high);
            close = glm::max(close, t_close_i);
            far = glm::min(far, t_far_i);
        }
        return close <= far;
    }

    void expandToInclude(const vertex vert) {
        minPoint = glm::min(minPoint, vert.position);
        maxPoint = glm::max(maxPoint, vert.position);
    }
};

inline std::ostream &operator<<(std::ostream &os, boundingVolume &bv) {
    os << "BoundingVolume(minPoint: " << bv.minPoint << ", maxPoint: " << bv.maxPoint << ")";
    return os;
}

class BVH : public hittable {
public:
    int maxDepth = 20;
    int minTriangles = 16;
    // array of all bounding volumes
    std::vector<boundingVolume> boxes;
    std::vector<triangle> triangles;

    BVH(const std::vector<triangle> &triangles) : triangles(triangles) {
        boxes.resize(1 << (maxDepth + 1));
        buildBoundingVolume(0, 0, static_cast<int>(triangles.size()));
        split(maxDepth, 0);
    }

    void buildBoundingVolume(int boxIndex, int start, int count) {
        boundingVolume &box = boxes[boxIndex];
        box.start = start;
        box.count = count;
        box.isLeaf = true;

        buildFromTriangles(box, start, count);
        findLongestAxis(box);
    }

    static auto getComparator(const axis ax) {
        switch (ax) {
            case X: return compX;
            case Y: return compY;
            case Z: return compZ;
        }
        return compX;
    }

    void split(const int depth, const int parentIndex) {
        // return if max split depth reached
        if (depth <= 0) return;

        const auto box = &boxes[parentIndex];
        if (box->count <= minTriangles) return;

        const int mid = box->count / 2;
        const auto comp = getComparator(box->longestAxis);
        std::nth_element(
            triangles.begin() + box->start,
            triangles.begin() + box->start + mid,
            triangles.begin() + box->start + box->count,
            comp
        );

        const int leftIndex = getLeftChildIndex(parentIndex);
        const int rightIndex = getRightChildIndex(parentIndex);

        buildBoundingVolume(leftIndex, box->start, mid);
        buildBoundingVolume(rightIndex, box->start + mid, box->count - mid);

        box->isLeaf = false;

        split(depth - 1, leftIndex);
        split(depth - 1, rightIndex);
    }

    bool hit(const ray &r, const interval ray_t, hitRecord &rec) const override {
        return hitRecursive(r, ray_t, rec, 0);
    }

    bool hitRecursive(const ray &r, const interval ray_t, hitRecord &rec, const int index) const {
        if (index < 0 || index >= static_cast<int>(boxes.size())) return false;
        const boundingVolume &box = boxes[index];
        if (!box.hit(r, ray_t, rec)) return false;

        if (box.isLeaf) return hitTriangles(r, ray_t, rec, box.start, box.count);
        // Internal Node
        hitRecord tempRec{};
        bool hitAnything = false;
        double closestSoFar = ray_t.max;

        int leftIndex = getLeftChildIndex(index);
        int rightIndex = getRightChildIndex(index);

        // Left child node
        if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, leftIndex)) {
            hitAnything = true;
            closestSoFar = tempRec.t;
            rec = tempRec;
        }

        // Right child Node
        if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, rightIndex)) {
            hitAnything = true;
            rec = tempRec;
        }

        return hitAnything;
    }

    bool hitTriangles(const ray &r, const interval ray_t, hitRecord &rec, const int start, const int count) const {
        hitRecord tempRec;
        bool hitAnything = false;
        double closestSoFar = ray_t.max;

        for (int i = start; i < start + count; i++) {
            if (triangles[i].hit(r, interval(ray_t.min, closestSoFar), tempRec)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }
        return hitAnything;
    }

    static bool compX(const triangle &a, const triangle &b) {
        return a.center.x < b.center.x;
    }

    static bool compY(const triangle &a, const triangle &b) {
        return a.center.y < b.center.y;
    }

    static bool compZ(const triangle &a, const triangle &b) {
        return a.center.z < b.center.z;
    }

private:
    static int getLeftChildIndex(const int i) {
        return 2 * i + 1;
    }

    static int getRightChildIndex(const int i) {
        return 2 * i + 2;
    }

    static void findLongestAxis(boundingVolume &box) {
        auto x_width = glm::abs(box.minPoint.x - box.maxPoint.x);
        auto y_width = glm::abs(box.minPoint.y - box.maxPoint.y);
        auto z_width = glm::abs(box.minPoint.z - box.maxPoint.z);

        if (x_width >= y_width && x_width >= z_width) box.longestAxis = X;
        else if (y_width >= x_width && y_width >= z_width) box.longestAxis = Y;
        else box.longestAxis = Z;
    }

    void buildFromTriangles(boundingVolume &box, int start, int count) {
        for (int i = start; i < start + count; i++) {
            const triangle tri = triangles[i];
            for (const auto &vertice: tri.vertices) {
                box.expandToInclude(vertice);
            }
        }
    }
};


#endif //BACHELORPROJECT_BOUNDINGVOLUME_H
