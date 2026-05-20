//
// Created by eliasw on 20.05.26.
//

#ifndef BACHELORPROJECT_BVH_H
#define BACHELORPROJECT_BVH_H

#include "../hittable.h"
#include "../triangleModel.h"
#include "boundingVolume.h"

class BVH : public hittable {
public:
    int maxDepth = 20;
    int minTriangles = 16;
    // array of all bounding volumes
    std::vector<boundingVolume> boxes;
    std::vector<triangle> triangles;

    explicit BVH(const std::vector<triangle> &triangles) : triangles(triangles) {
    }

    virtual ~BVH() = default;

    void build() {
        boxes.resize(1 << (maxDepth + 1));
        buildBoundingVolume(0, 0, static_cast<int>(triangles.size()));
        divide(maxDepth, 0);
    }

    virtual void buildBoundingVolume(int boxIndex, int start, int count) = 0;

    virtual void divide(int depth, int parentIndex) = 0;


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

        const int leftIndex = getLeftChildIndex(index);
        const int rightIndex = getRightChildIndex(index);

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

protected:
    static int getLeftChildIndex(const int i) {
        return 2 * i + 1;
    }

    static int getRightChildIndex(const int i) {
        return 2 * i + 2;
    }

    void buildFromTriangles(boundingVolume &box, const int start, const int count) const {
        box.count = 0;
        for (int i = start; i < start + count; i++) {
            const triangle tri = triangles[i];
            box.expandToInclude(tri);
        }
    }
};

#endif //BACHELORPROJECT_BVH_H
